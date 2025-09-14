#pragma once

#include <vector>
#include <cstdint>

namespace sanguosha {

class Player {
public:
    Player(uint32_t id, const std::string& username);
    
    uint32_t getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    
    // 血量相关
    uint32_t getHp() const { return hp_; }
    void setHp(uint32_t hp) { hp_ = hp; }
    void modifyHp(int delta);
    
    // 手牌管理
    const std::vector<uint32_t>& getHandCards() const { return handCards_; }
    void addCard(uint32_t cardId);
    bool removeCard(uint32_t cardId);
    
private:
    uint32_t id_;
    std::string username_;
    uint32_t hp_;
    uint32_t maxHp_;
    std::vector<uint32_t> handCards_;
};

}