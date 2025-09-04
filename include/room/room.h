#ifndef ROOM_ROOM_H
#define ROOM_ROOM_H

#include <vector>
#include <cstdint>
#include <memory>
#include <mutex>

namespace Sanguosha {
namespace Room {

class Room {
public:
    explicit Room(uint32_t id) : id_(id) {}
    
    uint32_t id() const { return id_; }
    
    // 玩家管理
    bool addPlayer(uint32_t playerId);
    bool removePlayer(uint32_t playerId);
    const std::vector<uint32_t>& players() const { return players_; }
    
    // 房间状态
    enum class State {
        WAITING,    // 等待玩家加入
        CHOOSING,   // 选将阶段
        PLAYING,    // 游戏中
        ENDED       // 结束
    };
    
    State state() const { return state_; }
    void setState(State state) { state_ = state; }
    
    // 状态转换方法
    bool startChoosing();  // 开始选将
    bool startGame();      // 开始游戏
    bool endGame();        // 结束游戏
    
    // 房间信息
    uint32_t playerCount() const { return players_.size(); }

private:
    uint32_t id_;
    std::vector<uint32_t> players_;
    State state_ = State::WAITING;
    mutable std::mutex mutex_;
};

} // namespace Room
} // namespace Sanguosha

#endif // ROOM_ROOM_H