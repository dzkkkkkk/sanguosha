#pragma once

#include <vector>
#include <cstdint>
#include <mutex>

namespace Sanguosha {
namespace Room {

class Room {
public:
    enum class State { WAITING, PLAYING };
    
    explicit Room(uint32_t id) : id_(id) {}
    
    // 简化：只支持2人游戏
    bool addPlayer(uint32_t playerId);
    bool removePlayer(uint32_t playerId);
    
    bool startGame();
    
    // 简化访问方法
    uint32_t playerCount() const { return players_.size(); }
    uint32_t id() const { return id_; }
    State state() const { return state_; }
    
    const std::vector<uint32_t>& getPlayers() const { return players_; }

private:
    uint32_t id_;
    std::vector<uint32_t> players_;
    State state_ = State::WAITING;
    std::mutex mutex_;
};

} // namespace Room
} // namespace Sanguosha