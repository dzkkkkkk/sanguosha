#include "game/game_instance.h"
#include "game/player.h"
#include "room/room_manager.h" // 添加包含
#include <random>
#include <algorithm>
#include "network/server.h" // 添加server.h包含

namespace sanguosha {

// 修改构造函数初始化列表 - 使用正确的前向声明
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

// 修改 processTurn 函数中的 broadcastGameState 调用
void GameInstance::processTurn(uint32_t playerId) {
    currentPlayer_ = playerId;
    
    // 摸牌阶段：摸2张牌
    sanguosha::GameMessage drawMessage;
    drawMessage.set_type(sanguosha::GAME_STATE);
    auto* gameState = drawMessage.mutable_game_state();
    gameState->set_current_player(currentPlayer_);
    gameState->set_phase(sanguosha::DRAW_PHASE);
    
    // 给当前玩家发2张牌
    auto& playerState = playerStates_[currentPlayer_];
    for (int i = 0; i < 2 && !deck_.empty(); i++) {
        uint32_t card = deck_.back();
        deck_.pop_back();
        playerState.add_hand_cards(card);
    }
    
    // 复制玩家状态到gameState
    for (const auto& pair : playerStates_) {
        PlayerState* ps = gameState->add_players();
        ps->CopyFrom(pair.second);
    }
    
    // 广播游戏状态 - 修复参数类型
    broadcastGameState(*gameState);
    
    // 进入出牌阶段
    gameState->set_phase(sanguosha::PLAY_PHASE);
    gameState->set_game_log("玩家 " + std::to_string(currentPlayer_) + " 的回合开始");
    // 广播游戏状态 - 修复参数类型
    broadcastGameState(*gameState);
}

// 修改 processPlayerAction 函数中的 broadcastGameState 调用
bool GameInstance::processPlayerAction(uint32_t playerId, const GameAction& action) {

    // 检查玩家是否已死亡
    auto playerIt = playerStates_.find(playerId);
    if (playerIt == playerStates_.end() || playerIt->second.hp() <= 0) {
        std::cerr << "Player " << playerId << " is dead or not found, ignoring action" << std::endl;
        return false;
    }
    
    // 检查游戏是否已结束
    if (gameOver_) {
        std::cerr << "Game is over, ignoring action" << std::endl;
        return false;
    }

    if (playerId != currentPlayer_) {
        return false; // 不是当前回合玩家
    }
    
    switch (action.type()) {
        case sanguosha::ACTION_PLAY_CARD:
            // 处理出牌逻辑
    if (action.card_id() == sanguosha::CARD_ATTACK && action.target_player() != 0) {
        // 从手牌中移除使用的牌
        auto& playerState = playerStates_[playerId];
        for (int i = 0; i < playerState.hand_cards_size(); i++) {
            if (playerState.hand_cards(i) == action.card_id()) {
                playerState.mutable_hand_cards()->erase(playerState.hand_cards().begin() + i);
                break;
            }
        }
        
        resolveAttack(playerId, action.target_player());
        
        // 修复：出牌后不结束回合，只更新状态
        sanguosha::GameMessage message;
        message.set_type(sanguosha::GAME_STATE);
        auto* gameState = message.mutable_game_state();
        gameState->set_current_player(currentPlayer_);
        gameState->set_phase(sanguosha::PLAY_PHASE);
        gameState->set_game_log("玩家 " + std::to_string(playerId) + " 使用了杀");
        
        // 复制玩家状态
        for (const auto& pair : playerStates_) {
            PlayerState* ps = gameState->add_players();
            ps->CopyFrom(pair.second);
        }
        
        broadcastGameState(*gameState);
    } else if (action.card_id() == sanguosha::CARD_HEAL) {
                // 处理桃：给自己加血
                auto& playerState = playerStates_[playerId];
                if (playerState.hp() < playerState.max_hp()) {
                    playerState.set_hp(playerState.hp() + 1);
                    
                    // 从手牌中移除使用的牌
                    for (int i = 0; i < playerState.hand_cards_size(); i++) {
                        if (playerState.hand_cards(i) == action.card_id()) {
                            playerState.mutable_hand_cards()->erase(playerState.hand_cards().begin() + i);
                            break;
                        }
                    }
                    
                    // 广播加血信息
                    sanguosha::GameMessage message;
                    message.set_type(sanguosha::GAME_STATE);
                    auto* gameState = message.mutable_game_state();
                    gameState->set_current_player(currentPlayer_);
                    gameState->set_phase(sanguosha::PLAY_PHASE);
                    gameState->set_game_log("玩家 " + std::to_string(playerId) + " 使用了桃，恢复1点体力");
                    
                    // 复制玩家状态
                    for (const auto& pair : playerStates_) {
                        PlayerState* ps = gameState->add_players();
                        ps->CopyFrom(pair.second);
                    }
                    
                    // 修复参数类型
                    broadcastGameState(*gameState);
                }
            }
            break;
            
        case sanguosha::ACTION_END_TURN:
    // 结束回合，切换到下一个玩家
    currentPlayer_ = getNextPlayer();
    
    // 发送回合结束通知
    sanguosha::GameMessage message;
    message.set_type(sanguosha::GAME_STATE);
    auto* gameState = message.mutable_game_state();
    gameState->set_current_player(currentPlayer_);
    gameState->set_phase(sanguosha::PLAY_PHASE);
    gameState->set_game_log("玩家 " + std::to_string(playerId) + " 结束了回合");
    
    // 复制玩家状态
    for (const auto& pair : playerStates_) {
        PlayerState* ps = gameState->add_players();
        ps->CopyFrom(pair.second);
    }
    
    broadcastGameState(*gameState);
    
    // 开始下一个玩家的回合
    processTurn(currentPlayer_);
    break;
    }
    
    return true;
}

void GameInstance::resolveAttack(uint32_t attacker, uint32_t target) {
    auto& targetState = playerStates_[target];
    
    // 检查目标玩家是否有闪
    bool hasDodge = false;
    for (int i = 0; i < targetState.hand_cards_size(); i++) {
        if (targetState.hand_cards(i) == sanguosha::CARD_DEFEND) {
            hasDodge = true;
            // 使用闪：从手牌中移除
            targetState.mutable_hand_cards()->erase(targetState.hand_cards().begin() + i);
            break;
        }
    }
    
    if (!hasDodge) {
        // 没有闪，扣血
        targetState.set_hp(targetState.hp() - 1);
        
        // 检查目标玩家是否死亡
        if (targetState.hp() <= 0) {
            // 玩家死亡，检查游戏是否结束
            if (checkGameOver()) {
                handleGameOver();
                return; // 游戏结束，不再继续处理
            }
            
            // 添加死亡玩家状态更新
            sanguosha::GameMessage deathMessage;
            deathMessage.set_type(sanguosha::GAME_STATE);
            auto* deathState = deathMessage.mutable_game_state();
            deathState->set_current_player(currentPlayer_);
            deathState->set_phase(sanguosha::PLAY_PHASE);
            deathState->set_game_log("玩家 " + std::to_string(target) + " 死亡");
            
            // 复制玩家状态
            for (const auto& pair : playerStates_) {
                PlayerState* ps = deathState->add_players();
                ps->CopyFrom(pair.second);
            }
            
            broadcastGameState(*deathState);
        }
    }
    
    // 广播游戏状态
    sanguosha::GameMessage message;
    message.set_type(sanguosha::GAME_STATE);
    auto* gameState = message.mutable_game_state();
    gameState->set_current_player(currentPlayer_);
    gameState->set_phase(sanguosha::PLAY_PHASE);
    if (hasDodge) {
        gameState->set_game_log("玩家 " + std::to_string(target) + " 使用了闪，抵消了杀");
    } else {
        gameState->set_game_log("玩家 " + std::to_string(target) + " 没有闪，受到1点伤害");
    }
    
    // 复制玩家状态
    for (const auto& pair : playerStates_) {
        PlayerState* ps = gameState->add_players();
        ps->CopyFrom(pair.second);
    }
    
    broadcastGameState(*gameState);
    
    // 检查游戏结束
    if (checkGameOver()) {
        handleGameOver();
    }
}

void GameInstance::broadcastGameState(const sanguosha::GameState& gameState) {
    roomManager_.broadcastMessage(roomId_, sanguosha::GAME_STATE, gameState, server_);
}

uint32_t GameInstance::getNextPlayer() {
    if (playerStates_.empty()) {
        return 0; // 但理论上不会为空，添加保护
    }
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

void GameInstance::handleGameOver() {
    // 确定胜利者
    winnerId_ = 0;
    for (const auto& pair : playerStates_) {
        if (pair.second.hp() > 0) {
            winnerId_ = pair.first;
            break;
        }
    }
    
    // 发送游戏结束消息
    sanguosha::GameMessage message;
    message.set_type(sanguosha::GAME_OVER);
    auto* gameOver = message.mutable_game_over();
    gameOver->set_winner_id(winnerId_);
    
    // 广播游戏结束
    roomManager_.broadcastMessage(roomId_, sanguosha::GAME_OVER, *gameOver, server_);
    
    gameOver_ = true;
}

bool GameInstance::isGameOver() const {
    return gameOver_;
}

uint32_t GameInstance::getWinner() const {
    // 这里需要实现获取胜利者的逻辑
    // 假设我们在handleGameOver中已经确定了胜利者
    // 你可能需要在类中添加一个winnerId_成员变量来存储胜利者ID
    for (const auto& pair : playerStates_) {
        if (pair.second.hp() > 0) {
            return pair.first; // 返回第一个存活的玩家作为胜利者
        }
    }
    return 0; // 如果没有存活的玩家，返回0
}

} // namespace sanguosha