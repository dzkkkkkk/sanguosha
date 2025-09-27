#include "network/session.h"
#include "network/message_codec.h"
#include "room/room_manager.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "room/room.h" // 添加room.h包含
#include "network/server.h" // 添加server.h包含
#include <iomanip>

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
            
            // 修复字节序处理
            uint32_t net_size;
            memcpy(&net_size, header_buffer_.data(), sizeof(uint32_t));
            expected_body_size_ = ntohl(net_size); // 正确转换网络字节序到主机字节序
            
            // 准备读取消息体
            body_buffer_.resize(expected_body_size_);
            doReadBody();
        });
}

void Session::doReadBody() {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(body_buffer_, expected_body_size_),
        [this, self](boost::system::error_code ec, size_t bytes_transferred) {
            if (ec) {
                std::cerr << "Body read error: " << ec.message() 
                          << ", expected: " << expected_body_size_
                          << ", transferred: " << bytes_transferred << std::endl;
                return;
            }
            
            try {
                // 直接解析消息体，不使用MessageCodec::decode
                sanguosha::GameMessage msg;
                if (!msg.ParseFromArray(body_buffer_.data(), body_buffer_.size())) {
                    std::cerr << "Parse message body failed. Body size: " 
                              << body_buffer_.size() << std::endl;
                    // 打印前20字节的十六进制用于调试
                    std::cerr << "First 20 bytes (hex): ";
                    for (size_t i = 0; i < std::min(body_buffer_.size(), size_t(20)); ++i) {
                        std::cerr << std::hex << std::setw(2) << std::setfill('0') 
                                  << static_cast<int>(body_buffer_[i]) << " ";
                    }
                    std::cerr << std::dec << std::endl;
                    return;
                }
                
                // 根据消息类型处理
                switch (msg.type()) {
                    case sanguosha::LOGIN_REQUEST:
                        std::cout << "Processing login request" << std::endl;
                        handleLogin(msg.login_request());
                        break;
                    case sanguosha::HEARTBEAT:
                        handleHeartbeat(ec);
                        break;
                    case sanguosha::ROOM_REQUEST:
                        handleRoomRequest(msg.room_request());
                        break;
                    case sanguosha::ROOM_LIST_REQUEST:
                        handleRoomListRequest();
                        break;
                    default:
                        std::cerr << "Unknown message type: " << msg.type() << std::endl;
                }
                
                // 继续读取下一条消息
                doReadHeader();
            } catch (const std::exception& e) {
                std::cerr << "Process message error: " << e.what() 
                          << ", buffer size: " << body_buffer_.size()
                          << ", expected size: " << expected_body_size_ << std::endl;
            }
        });

    // 调试信息：打印接收到的数据（十六进制）
    std::cout << "Received body data (hex): ";
    for (auto byte : body_buffer_) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
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
    std::cout << "Login attempt: " << login.username() << std::endl;
    
    sanguosha::GameMessage response;
    response.set_type(sanguosha::LOGIN_RESPONSE);
    auto* login_res = response.mutable_login_response();
    
    // 简化登录：允许任何用户名登录，分配随机用户ID
    // 在实际应用中，这里应该验证用户名和密码
    playerId_ = 1000 + (rand() % 9000);
    login_res->set_success(true);
    login_res->set_user_id(playerId_);
    
    std::cout << "Login successful, user ID: " << playerId_ << std::endl;
    
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
    // 计算消息体大小
    size_t body_size = msg.ByteSizeLong();
    size_t total_size = 4 + body_size;
    
    std::vector<char> buffer(total_size);
    
    // 写入消息头（长度）- 使用网络字节序
    uint32_t net_size = htonl(static_cast<uint32_t>(body_size));
    memcpy(buffer.data(), &net_size, 4);
    
    // 写入消息体
    if (!msg.SerializeToArray(buffer.data() + 4, body_size)) {
        std::cerr << "Failed to serialize message" << std::endl;
        return;
    }
    
    // 发送完整消息
    boost::asio::async_write(socket_, boost::asio::buffer(buffer),
        [self = shared_from_this()](boost::system::error_code ec, size_t) {
            if (ec) {
                std::cerr << "Send failed: " << ec.message() << std::endl;
                // 这里可以添加重连或关闭连接逻辑
            }
        });
    
    // 调试信息：打印发送的数据（十六进制）
    std::cout << "Sent message data (hex): ";
    for (size_t i = 0; i < std::min(buffer.size(), size_t(20)); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::dec << "..." << std::endl;
}

void Session::handleRoomListRequest() {
    auto& roomMgr = Sanguosha::Room::RoomManager::Instance();
    
    sanguosha::GameMessage response;
    response.set_type(sanguosha::ROOM_LIST_RESPONSE);
    auto* roomListRes = response.mutable_room_list_response();
    
    // 获取所有房间信息
    std::lock_guard<std::mutex> lock(roomMgr.getMutex());
    for (const auto& [roomId, room] : roomMgr.getRooms()) {
        sanguosha::RoomInfo* roomInfo = roomListRes->add_rooms();
        roomInfo->set_room_id(room->id());
        roomInfo->set_current_players(room->playerCount());
        roomInfo->set_max_players(2);
        
        // 修复：使用完全限定的枚举名称
        roomInfo->set_status(room->state() == Sanguosha::Room::Room::State::WAITING ? 
                           sanguosha::WAITING : sanguosha::PLAYING);
        
        const auto& players = room->getPlayers();
        for (uint32_t playerId : players) {
            roomInfo->add_players(playerId);
        }
    }
    
    send(response);
}

} // namespace Network
} // namespace Sanguosha