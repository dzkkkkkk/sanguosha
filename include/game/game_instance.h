#pragma once

#include <vector>
#include <unordered_map>
#include <random>
#include "proto/sanguosha.pb.h"

namespace sanguosha {

class Room;
class Player;

class GameInstance {
public:
    explicit GameInstance(Room* room);
    ~GameInstance();

    // 开始1v1游戏
    void startGame();
    
    // 处理玩家操作
    bool processPlayerAction(uint32_t playerId, const GameAction& action);
    
    // 获取当前游戏状态
    GameState getGameState() const;

private:
    // 初始化牌堆
    void initDeck();
    
    // 分发起始手牌
    void dealInitialCards();
    
    // 处理回合
    void processTurn(uint32_t playerId);
    
    // 获取下一个玩家
    uint32_t getNextPlayer() const;
    
    // 检查游戏结束条件
    bool checkGameOver();
    
    // 解决攻击
    void resolveAttack(uint32_t attacker, uint32_t target);
    
    // 广播游戏状态
    void broadcastGameState();

    Room* room_;
    std::vector<uint32_t> deck_;
    std::unordered_map<uint32_t, PlayerState> playerStates_;
    uint32_t currentPlayer_;
    bool gameOver_;
    std::mt19937 rng_;
};

}