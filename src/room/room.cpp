#include "room/room.h"

namespace Sanguosha {
namespace Room {

bool Room::addPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查玩家是否已在房间中
    for (auto id : players_) {
        if (id == playerId) {
            return false;
        }
    }
    
    // 检查房间状态
    if (state_ != State::WAITING) {
        return false;
    }
    
    // 添加玩家（最大8人）
    if (players_.size() >= 8) {
        return false;
    }
    
    players_.push_back(playerId);
    return true;
}

bool Room::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找并移除玩家
    auto it = std::find(players_.begin(), players_.end(), playerId);
    if (it == players_.end()) {
        return false;
    }
    
    players_.erase(it);
    
    // 如果房间空则销毁
    if (players_.empty()) {
        // 注意：实际需要由RoomManager清理空房间
    }
    
    return true;
}

} // namespace Room
} // namespace Sanguosha