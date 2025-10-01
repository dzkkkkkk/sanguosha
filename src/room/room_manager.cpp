#include "room/room_manager.h"
#include "room/room.h"
#include "network/server.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace Sanguosha {
namespace Room {

// 静态成员初始化
boost::asio::io_context RoomManager::dummy_io_context_;

RoomManager::RoomManager() 
    : io_(nullptr),
      cleanupTimer_(std::make_unique<boost::asio::steady_timer>(dummy_io_context_)) {
}

RoomManager::~RoomManager() {
    if (cleanupTimer_) {
        boost::system::error_code ec;
        cleanupTimer_->cancel(ec);
    }
}

RoomManager& RoomManager::Instance() {
    static RoomManager instance;
    return instance;
}

uint32_t RoomManager::createRoom() {
    return createRoom({});
}

void RoomManager::setServer(Sanguosha::Network::Server& server) {
    serverPtr_ = &server;
}

uint32_t RoomManager::createRoom(const std::vector<uint32_t>& playerIds) {
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t roomId = nextRoomId_++;
    
    auto room = std::make_shared<Room>(roomId);
    for (auto playerId : playerIds) {
        room->addPlayer(playerId);
    }
    
    rooms_[roomId] = room;
    return roomId;
}

bool RoomManager::joinRoom(uint32_t roomId, uint32_t playerId) {
    std::shared_ptr<Room> room;
    bool shouldStartGame = false;
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "Attempting to join room " << roomId << " with player " << playerId << std::endl;
        
        auto it = rooms_.find(roomId);
        if (it == rooms_.end()) {
            std::cout << "Room not found: " << roomId << std::endl;
            return false;
        }
        
        room = it->second;
        bool success = room->addPlayer(playerId);
        std::cout << "Join room result: " << success << std::endl;
        
        // 检查是否需要开始游戏
        if (success && room->playerCount() == 2 && room->state() == Room::State::WAITING) {
            std::cout << "Room is full, will start game" << std::endl;
            shouldStartGame = true;
        }
        
        if (!success) return false;
    } // 释放锁后再开始游戏
    
    // 在锁外开始游戏，避免死锁
    if (shouldStartGame && serverPtr_ != nullptr) {
        room->startGame(*this, *serverPtr_);
    }
    
    return true;
}

bool RoomManager::leaveRoom(uint32_t roomId, uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) {
        return false;
    }
    return it->second->removePlayer(playerId);
}

// 修复：使用完整类型替代别名
std::shared_ptr<Room> RoomManager::getRoom(uint32_t roomId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    return it != rooms_.end() ? it->second : nullptr;
}

// 保持实现但已添加声明
uint32_t RoomManager::matchPlayers(const std::vector<uint32_t>& playerIds) {
    if (playerIds.empty()) return 0;
    
    // 1. 尝试找到合适的现有房间
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (auto& [id, room] : rooms_) {
            if (room->state() == Room::State::WAITING && 
                room->playerCount() < 8 && 
                room->playerCount() + playerIds.size() <= 8) {
                
                // 加入房间
                for (auto playerId : playerIds) {
                    room->addPlayer(playerId);
                }
                return id;
            }
        }
    }
    
    // 2. 没有合适房间则创建新房间
    return createRoom(playerIds);
}

void RoomManager::setIoContext(boost::asio::io_context& io) {
    io_ = &io;
    
    // 创建新的定时器
    cleanupTimer_ = std::make_unique<boost::asio::steady_timer>(*io_);
}

void RoomManager::startCleanupTask() {
    if (!io_ || !cleanupTimer_) return;
    
    cleanupTimer_->expires_after(std::chrono::minutes(5));
    cleanupTimer_->async_wait([this](const boost::system::error_code& ec) {
        if (!ec) {
            cleanupRooms();
            startCleanupTask();
        }
    });
}

void RoomManager::cleanupRooms() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.begin();
    while (it != rooms_.end()) {
        // 清理条件：房间为空
        if (it->second->playerCount() == 0) {
            it = rooms_.erase(it);
        } else {
            ++it;
        }
    }
}

void RoomManager::broadcastMessage(uint32_t roomId, sanguosha::MessageType type, const google::protobuf::Message& message, Network::Server& server) {
    std::vector<uint32_t> players;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = rooms_.find(roomId);
        if (it == rooms_.end()) {
            return;
        }
        players = it->second->getPlayers();
    } // 释放锁后再发送消息
    
    sanguosha::GameMessage gameMsg;
    gameMsg.set_type(type);
    
    // 根据不同的消息类型，设置对应的字段
    switch (type) {
        case sanguosha::GAME_STATE:
            if (message.GetTypeName() == "sanguosha.GameState") {
                gameMsg.mutable_game_state()->CopyFrom(dynamic_cast<const sanguosha::GameState&>(message));
            }
            break;
        case sanguosha::GAME_START:
            if (message.GetTypeName() == "sanguosha.GameStart") {
                gameMsg.mutable_game_start()->CopyFrom(dynamic_cast<const sanguosha::GameStart&>(message));
            }
            break;
        case sanguosha::ROOM_RESPONSE:
            if (message.GetTypeName() == "sanguosha.RoomResponse") {
                gameMsg.mutable_room_response()->CopyFrom(dynamic_cast<const sanguosha::RoomResponse&>(message));
            }
            break;
        default:
            std::cerr << "Unknown message type for broadcast: " << type << std::endl;
            return;
    }

    for (uint32_t playerId : players) {
        if (auto session = server.getSession(playerId)) {
            session->send(gameMsg);
        }
    }
}

} // namespace Room
} // namespace Sanguosha