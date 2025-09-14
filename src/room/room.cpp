#include "room/room.h"
#include <algorithm>

namespace Sanguosha {
namespace Room {

bool Room::addPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.size() >= 2) // 只支持2人游戏
        return false;
    
    players_.push_back(playerId);
    return true;
}

bool Room::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find(players_.begin(), players_.end(), playerId);
    if (it == players_.end())
        return false;
    
    players_.erase(it);
    return true;
}

bool Room::startGame() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.size() != 2) // 需要恰好2人才能开始游戏
        return false;
    
    state_ = State::PLAYING;
    return true;
}

} // namespace Room
} // namespace Sanguosha