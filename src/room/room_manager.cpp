#include <cmath>
#include <algorithm>
#include "room/room_manager.h"

// ELO匹配算法实现
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

// 创建房间并添加玩家
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