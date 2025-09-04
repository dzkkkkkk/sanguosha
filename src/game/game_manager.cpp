#include "game/game_manager.h"

namespace Sanguosha {
namespace Game {

GameManager& GameManager::Instance() {
    static GameManager instance;
    return instance;
}

GamePtr GameManager::createGame(Sanguosha::Room::RoomPtr room) {
    // 返回空指针
    return nullptr;
}

GamePtr GameManager::getGame(uint32_t roomId) {
    // 返回空的GamePtr
    return nullptr;
}

} // namespace Game
} // namespace Sanguosha