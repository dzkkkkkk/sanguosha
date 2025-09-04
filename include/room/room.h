#pragma once

#include <vector>
#include <mutex>

namespace Sanguosha {
namespace Room {

class Room {
public:
    enum class State { WAITING, CHOOSING, PLAYING, ENDED };
    
    explicit Room(uint32_t id);
    
    bool addPlayer(uint32_t playerId);
    bool removePlayer(uint32_t playerId);
    bool startChoosing();
    bool startGame();
    bool endGame();
    
    // 添加以下访问方法
    uint32_t playerCount() const { return players_.size(); }
    uint32_t id() const { return id_; }
    State state() const { return state_; }

private:
    uint32_t id_;
    std::vector<uint32_t> players_;
    State state_ = State::WAITING;
    std::mutex mutex_;
};

} // namespace Room
} // namespace Sanguosha