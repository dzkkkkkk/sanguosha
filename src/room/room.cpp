#include "room/room.h"
#include "game/game_instance.h" // 包含GameInstance头文件
#include "network/server.h"     // 包含Server头文件
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

bool Room::startGame(RoomManager& roomManager, Network::Server& server) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.size() != 2 || state_ != State::WAITING) { // 假设2人开始游戏
        return false;
    }
    state_ = State::PLAYING;
    
    // 核心：创建GameInstance！
    gameInstance_ = std::make_shared<sanguosha::GameInstance>(id_, roomManager, server);
    gameInstance_->startGame(players_); // 将本房间的玩家列表传入
    
    return true;
}

} // namespace Room
} // namespace Sanguosha