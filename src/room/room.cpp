#include <algorithm>  // 添加这行
#include "room/room.h"

namespace Sanguosha {
namespace Room {

bool Room::addPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != State::WAITING || players_.size() >= 2) {
        return false;
    }
    
    players_.push_back(playerId);
    return true;
}

bool Room::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(players_.begin(), players_.end(), playerId);
    if (it == players_.end()) {
        return false;
    }
    
    players_.erase(it);
    return true;
}

bool Room::startGame() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (state_ != State::WAITING || players_.size() != 2) {
        return false;
    }
    
    state_ = State::PLAYING;
    return true;
}

} // namespace Room
} // namespace Sanguosha