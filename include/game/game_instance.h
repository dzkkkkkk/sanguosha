#pragma once

#include <vector>
#include <unordered_map>
#include <random>
#include "proto/sanguosha.pb.h"

namespace sanguosha {

class Player;

class GameInstance {
public:
    explicit GameInstance(uint32_t roomId);
    
    // 开始1v1游戏
    void startGame(const std::vector<uint32_t>& playerIds);
    
    // 处理玩家操作
    bool processPlayerAction(uint32_t playerId, const GameAction& action);
    
    // 获取当前游戏状态
    GameState getGameState() const;

private:
    // 简化：移除牌堆初始化等复杂逻辑
    void initPlayerStates(const std::vector<uint32_t>& playerIds);
    
    // 广播游戏状态
    void broadcastGameState();

    uint32_t roomId_;
    std::unordered_map<uint32_t, PlayerState> playerStates_;
    uint32_t currentPlayer_;
    bool gameOver_;
};

}