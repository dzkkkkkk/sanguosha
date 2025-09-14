#include "game/game_instance.h"
#include "game/player.h"
//#include "network/connection.h"
#include "room/room.h"

namespace sanguosha {

GameInstance::GameInstance(Room* room) 
    : room_(room), currentPlayer_(0), gameOver_(false) {
    // 初始化随机数生成器
    std::random_device rd;
    rng_.seed(rd());
    
    // 初始化玩家状态
    for (auto player : room_->getPlayers()) {
        PlayerState state;
        state.set_player_id(player->getId());
        state.set_username(player->getUsername());
        state.set_hp(4);
        state.set_max_hp(4);
        playerStates_[player->getId()] = state;
    }
}

void GameInstance::startGame() {
    // 1. 初始化牌堆
    initDeck();
    
    // 2. 分发起始手牌
    dealInitialCards();
    
    // 3. 决定先手玩家
    auto players = room_->getPlayers();
    currentPlayer_ = players[0]->getId();
    
    // 4. 通知游戏开始
    GameStart startMsg;
    startMsg.set_room_id(room_->getId());
    for (auto player : players) {
        startMsg.add_player_ids(player->getId());
    }
    
    room_->broadcastMessage(GAME_START, startMsg);
    
    // 5. 开始第一个回合
    processTurn(currentPlayer_);
}

void GameInstance::initDeck() {
    deck_.clear();
    
    // 简单卡牌配置：30张杀，15张闪，8张桃
    for (int i = 0; i < 30; i++) deck_.push_back(static_cast<uint32_t>(CARD_ATTACK));
    for (int i = 0; i < 15; i++) deck_.push_back(static_cast<uint32_t>(CARD_DEFEND));
    for (int i = 0; i < 8; i++) deck_.push_back(static_cast<uint32_t>(CARD_HEAL));
    
    // 洗牌
    std::shuffle(deck_.begin(), deck_.end(), rng_);
}

void GameInstance::dealInitialCards() {
    auto players = room_->getPlayers();
    
    // 每个玩家发4张牌
    for (auto player : players) {
        auto& state = playerStates_[player->getId()];
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
        case ACTION_PLAY_CARD:
            // 处理出牌逻辑
            if (action.card_id() == CARD_ATTACK && action.target_player() != 0) {
                resolveAttack(playerId, action.target_player());
            }
            break;
            
        case ACTION_END_TURN:
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
    
    room_->broadcastMessage(GAME_STATE, state);
}

}