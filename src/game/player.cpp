// src/game/player.cpp
#include "game/player.h"

namespace sanguosha {

Player::Player(uint32_t id, const std::string& username)
    : id_(id), username_(username) {}

uint32_t Player::getId() const {
    return id_;
}

const std::string& Player::getUsername() const {
    return username_;
}

} // namespace sanguosha