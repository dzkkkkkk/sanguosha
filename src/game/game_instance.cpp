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
    
    // 创建需要响应的游戏状态
    sanguosha::GameMessage message;
    message.set_type(sanguosha::GAME_STATE);
    auto* gameState = message.mutable_game_state();
    gameState->set_current_player(target); // 设置当前玩家为目标玩家
    gameState->set_phase(sanguosha::PLAY_PHASE);
    gameState->set_game_log("玩家 " + std::to_string(attacker) + " 对您使用了杀，请使用闪响应");
    
    // 复制玩家状态
    for (const auto& pair : playerStates_) {
        PlayerState* ps = gameState->add_players();
        ps->CopyFrom(pair.second);
    }
    
    // 只发送给目标玩家
    if (auto session = server_.getSession(target)) {
        session->send(message);
    }
    
    // 设置超时计时器，如果没有响应则自动处理
    // 这里需要实现超时逻辑，简化版可以先不处理
}

void GameInstance::broadcastGameState(const sanguosha::GameState& gameState) {
    roomManager_.broadcastMessage(roomId_, sanguosha::GAME_STATE, gameState, server_);
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

void GameInstance::handleGameOver() {
    // 确定胜利者
    uint32_t winnerId = 0;
    for (const auto& pair : playerStates_) {
        if (pair.second.hp() > 0) {
            winnerId = pair.first;
            break;
        }
    }
    
    // 发送游戏结束消息
    sanguosha::GameMessage message;
    message.set_type(sanguosha::GAME_OVER);
    auto* gameOver = message.mutable_game_over();
    gameOver->set_winner_id(winnerId);
    
    // 广播游戏结束
    roomManager_.broadcastMessage(roomId_, sanguosha::GAME_OVER, *gameOver, server_);
    
    gameOver_ = true;
}

} // namespace sanguosha