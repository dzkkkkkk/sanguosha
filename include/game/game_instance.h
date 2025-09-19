#pragma once

#include <vector>
#include <unordered_map>
#include <random>
#include "sanguosha.pb.h"

// 前向声明，避免包含player.h
namespace sanguosha {
class Player;
}

// 前向声明RoomManager
namespace Sanguosha {
namespace Room {
class RoomManager;
}
}

// 前向声明Server - 添加正确的命名空间
namespace Sanguosha {
namespace Network {
class Server;
}
}

namespace sanguosha {

class GameInstance {
public:
    // 修改构造函数，使用正确的前向声明
    explicit GameInstance(uint32_t roomId, Sanguosha::Room::RoomManager& roomManager, Sanguosha::Network::Server& server);

    // 开始1v1游戏
    void startGame(const std::vector<uint32_t>& playerIds);

    // 处理玩家操作
    bool processPlayerAction(uint32_t playerId, const GameAction& action);

    // 获取当前游戏状态
    GameState getGameState() const;

private:
    // 添加缺失的方法声明
    void initDeck();
    void dealInitialCards();
    void processTurn(uint32_t playerId);
    void resolveAttack(uint32_t attacker, uint32_t target);
    void broadcastGameState();
    uint32_t getNextPlayer();
    bool checkGameOver();

    // 添加必要的成员变量
    std::vector<uint32_t> deck_;
    std::mt19937 rng_;

    uint32_t roomId_;
    Sanguosha::Room::RoomManager& roomManager_; // 添加RoomManager引用
    Sanguosha::Network::Server& server_; // 使用正确的前向声明
    std::unordered_map<uint32_t, PlayerState> playerStates_;
    uint32_t currentPlayer_;
    bool gameOver_;
};

} // namespace sanguosha