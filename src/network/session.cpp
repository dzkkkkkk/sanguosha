#include "network/session.h"
#include "network/message_codec.h"
#include "room/room_manager.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "room/room.h" // 添加room.h包含
#include "network/server.h" // 添加server.h包含

using boost::asio::ip::tcp;
using boost::asio::steady_timer;

namespace Sanguosha {
namespace Network {

Session::Session(tcp::socket socket, Server& server)
    : socket_(std::move(socket)),
      server_(server),
      heartbeat_timer_(socket_.get_executor()) {
}

Session::~Session() {
    if (playerId_ != 0) {
        server_.unregisterSession(playerId_);
    }
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
                    case sanguosha::ROOM_REQUEST:
                        handleRoomRequest(msg.room_request());
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

void Session::handleHeartbeat(const boost::system::error_code& ec) {
    if (ec) {
        // 如果有错误，记录日志但不中断连接
        std::cerr << "Heartbeat error: " << ec.message() << std::endl;
        return;
    }
    
    // 重置心跳计时器
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
    
    // 可以在这里添加其他心跳处理逻辑
    std::cout << "Heartbeat received from player: " << playerId_ << std::endl;
}

void Session::handleLogin(const sanguosha::LoginRequest& login) {
    std::cout << "Login: " << login.username() << std::endl;
    
    sanguosha::GameMessage response;
    response.set_type(sanguosha::LOGIN_RESPONSE);
    auto* login_res = response.mutable_login_response();
    
    // 简化登录：直接分配用户ID
    playerId_ = 1000 + rand() % 9000; // 随机生成用户ID
    login_res->set_success(true);
    login_res->set_user_id(playerId_);
    
    // 注册会话到服务器
    server_.registerSession(playerId_, shared_from_this());
    
    send(response);
}

void Session::handleRoomRequest(const sanguosha::RoomRequest& request) {
    sanguosha::GameMessage response;
    response.set_type(sanguosha::ROOM_RESPONSE);
    auto* room_res = response.mutable_room_response();
    
    auto& roomMgr = Sanguosha::Room::RoomManager::Instance();
    
    switch (request.action()) {
        case sanguosha::CREATE_ROOM: {
            uint32_t roomId = roomMgr.createRoom();
            if (roomMgr.joinRoom(roomId, playerId_)) {
                room_res->set_success(true);
                room_res->mutable_room_info()->set_room_id(roomId);
            } else {
                room_res->set_success(false);
                room_res->set_error_message("Create room failed");
            }
            break;
        }
        case sanguosha::JOIN_ROOM: {
            if (roomMgr.joinRoom(request.room_id(), playerId_)) {
                room_res->set_success(true);
                // 检查是否可以开始游戏
                auto room = roomMgr.getRoom(request.room_id());
                if (room && room->playerCount() == 2) {
                    // 修正：传递roomManager和server引用
                    room->startGame(roomMgr, server_);
                }
            } else {
                room_res->set_success(false);
                room_res->set_error_message("Join room failed");
            }
            break;
        }
        case sanguosha::START_GAME: {
            room_res->set_success(false);
            room_res->set_error_message("Start game is automatically handled");
            break;
        }
    }
    
    send(response);
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