#pragma once

#include <vector>
#include <cstdint>
#include <mutex>
#include <memory>
#include "network/server.h" // 包含Server的头文件
#include "room/room_manager.h" // 包含RoomManager的头文件
#include "game/game_instance.h" // 包含GameInstance的头文件

// 前向声明
namespace sanguosha {
    class GameInstance;
}

namespace Sanguosha {
namespace Room {

class Room {
public:
    enum class State { WAITING, PLAYING };
    
    explicit Room(uint32_t id) : id_(id), state_(State::WAITING) {}
    
    // 简化：只支持2人游戏
    bool addPlayer(uint32_t playerId);
    bool removePlayer(uint32_t playerId);
    
    // 修改startGame签名
    bool startGame(RoomManager& roomManager, Network::Server& server);
    
    // 简化访问方法
    uint32_t playerCount() const { return players_.size(); }
    uint32_t id() const { return id_; }
    State state() const { return state_; }
    
    const std::vector<uint32_t>& getPlayers() const { return players_; }
    
    bool isPlaying() const { return state_ == State::PLAYING; }
    std::shared_ptr<sanguosha::GameInstance> getGameInstance() const { return gameInstance_; }

private:
    uint32_t id_;
    std::vector<uint32_t> players_;
    State state_;
    std::mutex mutex_;
    
    std::shared_ptr<sanguosha::GameInstance> gameInstance_; // 使用shared_ptr管理GameInstance
};

} // namespace Room
} // namespace Sanguosha