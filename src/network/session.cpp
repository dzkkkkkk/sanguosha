#include "network/session.h"
#include "network/message_codec.h"
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::steady_timer;

namespace Sanguosha {
namespace Network {

Session::Session(tcp::socket socket)
    : socket_(std::move(socket)),
      heartbeat_timer_(socket_.get_executor()) {
}

void Session::start() {
    startHeartbeat();
    doReadHeader();
}

void Session::startHeartbeat() {
    // 设置心跳计时器
    heartbeat_timer_.expires_after(std::chrono::seconds(HEARTBEAT_INTERVAL));
    heartbeat_timer_.async_wait(
        [self = shared_from_this()](const boost::system::error_code& ec) {
            if (!ec) {
                // 发送心跳包
                sanguosha::GameMessage msg;
                msg.set_type(sanguosha::HEARTBEAT);
                self->send(msg);
                
                // 重新设置计时器
                self->startHeartbeat();
            }
        });
}

void Session::doReadHeader() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(header_buffer_, MessageCodec::HEADER_LENGTH),
        [this, self](boost::system::error_code ec, size_t) {
            if (ec) {
                if (ec != boost::asio::error::eof) {
                    std::cerr << "Header read error: " << ec.message() << std::endl;
                }
                return;
            }
            
            // 解析头部大小
            memcpy(&expected_body_size_, header_buffer_.data(), sizeof(uint32_t));
            expected_body_size_ = ntohl(expected_body_size_);
            
            // 准备读取消息体
            body_buffer_.resize(expected_body_size_);
            doReadBody();
        });
}

void Session::doReadBody() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(body_buffer_, expected_body_size_),
        [this, self](boost::system::error_code ec, size_t) {
            if (ec) {
                std::cerr << "Body read error: " << ec.message() << std::endl;
                return;
            }
            
            try {
                // 解析消息
                auto msg = MessageCodec::decode(body_buffer_);
                
                // 根据消息类型处理
                switch (msg.type()) {
                    case sanguosha::LOGIN_REQUEST:
                        handleLogin(msg.login_request());
                        break;
                    case sanguosha::HEARTBEAT:
                        handleHeartbeat(boost::system::error_code());
                        break;
                    default:
                        std::cerr << "Unknown message type: " << msg.type() << std::endl;
                }
                
                // 继续读取下一条消息
                doReadHeader();
            } catch (const std::exception& e) {
                std::cerr << "Decode error: " << e.what() << std::endl;
            }
        });
}

void Session::handleLogin(const sanguosha::LoginRequest& login) {
    std::cout << "Login request from: " << login.username() << std::endl;
    
    sanguosha::GameMessage response;
    response.set_type(sanguosha::LOGIN_RESPONSE);
    auto* login_res = response.mutable_login_response();
    
    // 简单验证：用户名为"test"，密码为"123"
    if (login.username() == "test" && login.password() == "123") {
        login_res->set_success(true);
        login_res->set_user_id(1001); // 模拟用户ID
    } else {
        login_res->set_success(false);
        login_res->set_error_message("Invalid username or password");
    }
    
    send(response);
}

void Session::handleHeartbeat(const boost::system::error_code& ec) {
    if (!ec) {
        // 处理心跳响应
        // 这里可以添加超时检测逻辑
    }
}

void Session::send(const sanguosha::GameMessage& msg) {
    auto buffer = MessageCodec::encode(msg);
    boost::asio::async_write(socket_, boost::asio::buffer(buffer),
        [self = shared_from_this()](boost::system::error_code ec, size_t) {
            if (ec) {
                std::cerr << "Send failed: " << ec.message() << std::endl;
                // 这里可以添加重连或关闭连接逻辑
            }
        });
}

} // namespace Network
} // namespace Sanguosha