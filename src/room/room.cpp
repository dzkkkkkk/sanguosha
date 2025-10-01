#include "room/room.h"
#include "room/room_manager.h"
#include "game/game_instance.h"
#include "network/server.h"
#include <algorithm>

namespace Sanguosha {
namespace Room {

Room::Room(uint32_t id) : id_(id), state_(State::WAITING) {}

bool Room::addPlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.size() >= 2)
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

// room.cpp - 修改startGame函数
bool Room::startGame(RoomManager& roomManager, Sanguosha::Network::Server& server) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.size() != 2 || state_ != State::WAITING) {
        return false;
    }
    state_ = State::PLAYING;
    
    gameInstance_ = std::make_shared<sanguosha::GameInstance>(id_, roomManager, server);
    gameInstance_->startGame(players_);
    
    // 广播游戏开始消息
    sanguosha::GameStart gameStartMsg;
    gameStartMsg.set_room_id(id_); // 确保设置正确的房间ID
    for (auto playerId : players_) {
        gameStartMsg.add_player_ids(playerId);
    }
    
    // 使用正确的消息类型
    roomManager.broadcastMessage(id_, sanguosha::GAME_START, gameStartMsg, server);
    
    return true;
}

uint32_t Room::playerCount() const { return players_.size(); }
uint32_t Room::id() const { return id_; }
Room::State Room::state() const { return state_; }

const std::vector<uint32_t>& Room::getPlayers() const { return players_; }

bool Room::isPlaying() const { return state_ == State::PLAYING; }
std::shared_ptr<sanguosha::GameInstance> Room::getGameInstance() const { return gameInstance_; }

} // namespace Room
} // namespace Sanguosha