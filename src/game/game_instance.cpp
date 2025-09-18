#include "game/game_instance.h"
#include "game/player.h"
#include "room/room_manager.h" // 添加包含
#include <random>
#include <algorithm>

namespace sanguosha {

// 修改构造函数初始化列表
GameInstance::GameInstance(uint32_t roomId, Sanguosha::Room::RoomManager& roomManager, Sanguosha::Network::Server& server)
    : roomId_(roomId), roomManager_(roomManager), server_(server), currentPlayer_(0), gameOver_(false) {
    // 初始化随机数生成器
    std::random_device rd;
    rng_.seed(rd());
}

void GameInstance::startGame(const std::vector<uint32_t>& playerIds) {
    // 1. 初始化玩家状态
    for (auto playerId : playerIds) {
        PlayerState state;
        state.set_player_id(playerId);
        state.set_hp(4);
        state.set_max_hp(4);
        playerStates_[playerId] = state;
    }
    
    // 2. 初始化牌堆
    initDeck();
    
    // 3. 分发起始手牌
    dealInitialCards();
    
    // 4. 决定先手玩家
    currentPlayer_ = playerIds[0];
    
    // 5. 开始第一个回合
    processTurn(currentPlayer_);
}

void GameInstance::initDeck() {
    deck_.clear();
    
    // 简单卡牌配置：30张杀，15张闪，8张桃
    for (int i = 0; i < 30; i++) deck_.push_back(static_cast<uint32_t>(sanguosha::CARD_ATTACK));
    for (int i = 0; i < 15; i++) deck_.push_back(static_cast<uint32_t>(sanguosha::CARD_DEFEND));
    for (int i = 0; i < 8; i++) deck_.push_back(static_cast<uint32_t>(sanguosha::CARD_HEAL));
    
    // 洗牌
    std::shuffle(deck_.begin(), deck_.end(), rng_);
}

void GameInstance::dealInitialCards() {
    // 每个玩家发4张牌
    for (const auto& pair : playerStates_) {
        uint32_t playerId = pair.first;
        auto& state = playerStates_[playerId];
        for (int i = 0; i < 4 && !deck_.empty(); i++) {
            uint32_t card = deck_.back();
            deck_.pop_back();
            state.add_hand_cards(card);
        }
    }
}

void GameInstance::processTurn(uint32_t playerId) {
    currentPlayer_ = playerId;
    
    // 通知客户端回合开始
    broadcastGameState();
    
    // 在实际实现中，这里会等待玩家操作
    // 通过processPlayerAction方法处理玩家操作
}

bool GameInstance::processPlayerAction(uint32_t playerId, const GameAction& action) {
    if (playerId != currentPlayer_) {
        return false; // 不是当前回合玩家
    }
    
    switch (action.type()) {
        case sanguosha::ACTION_PLAY_CARD:
            // 处理出牌逻辑
            if (action.card_id() == sanguosha::CARD_ATTACK && action.target_player() != 0) {
                resolveAttack(playerId, action.target_player());
            }
            break;
            
        case sanguosha::ACTION_END_TURN:
            // 结束回合，切换到下一个玩家
            currentPlayer_ = getNextPlayer();
            processTurn(currentPlayer_);
            break;
    }
    
    // 检查游戏是否结束
    if (checkGameOver()) {
        // 处理游戏结束逻辑
    }
    
    return true;
}

void GameInstance::resolveAttack(uint32_t attacker, uint32_t target) {
    auto& targetState = playerStates_[target];
    
    // 简化版：直接扣血
    targetState.set_hp(targetState.hp() - 1);
    
    // 广播更新后的游戏状态
    broadcastGameState();
}

void GameInstance::broadcastGameState() {
    GameState state;
    state.set_current_player(currentPlayer_);
    
    for (const auto& pair : playerStates_) {
        PlayerState* playerState = state.add_players();
        playerState->CopyFrom(pair.second);
    }
    
    // 调用roomManager的广播方法，并传入server_
    roomManager_.broadcastMessage(roomId_, sanguosha::GAME_STATE, state, server_);
}

uint32_t GameInstance::getNextPlayer() {
    // 简单实现：按玩家ID顺序获取下一个玩家
    auto it = playerStates_.find(currentPlayer_);
    if (it != playerStates_.end()) {
        auto nextIt = std::next(it);
        if (nextIt != playerStates_.end()) {
            return nextIt->first;
        }
    }
    return playerStates_.begin()->first; // 如果是最后一个玩家，回到第一个玩家
}

bool GameInstance::checkGameOver() {
    // 简单实现：检查是否有玩家生命值为0
    for (const auto& pair : playerStates_) {
        if (pair.second.hp() <= 0) {
            return true;
        }
    }
    return false;
}

} // namespace sanguosha