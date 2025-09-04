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

bool Room::startChoosing() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != State::WAITING) {
        return false;
    }
    
    // 至少需要2名玩家
    if (players_.size() < 2) {
        return false;
    }
    
    state_ = State::CHOOSING;
    return true;
}

bool Room::startGame() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != State::CHOOSING) {
        return false;
    }
    
    // 检查所有玩家是否已选将
    // TODO: 实现选将检查逻辑
    // 示例：假设所有玩家都已选将
    bool allPlayersChosen = true; // 这里需要根据实际情况实现
    
    if (!allPlayersChosen) {
        return false;
    }
    
    state_ = State::PLAYING;
    return true;
}

bool Room::endGame() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != State::PLAYING) {
        return false;
    }
    
    state_ = State::ENDED;
    // TODO: 结算游戏结果
    // 示例：这里可以添加一些游戏结果的结算逻辑
    // 例如：记录玩家得分、发送游戏结果通知等
    
    return true;
}

} // namespace Room
} // namespace Sanguosha