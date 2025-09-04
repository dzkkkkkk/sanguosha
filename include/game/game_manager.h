#ifndef GAME_GAME_MANAGER_H
#define GAME_GAME_MANAGER_H

#include <map>
#include <memory>

// 前向声明
namespace Sanguosha {
namespace Room {
    class Room;
    using RoomPtr = std::shared_ptr<Room>;
}
}

namespace Sanguosha {
namespace Game {

class GameInstance;
using GamePtr = std::shared_ptr<GameInstance>;

class GameManager {
public:
    static GameManager& Instance();
    
    // 创建游戏实例
    GamePtr createGame(Sanguosha::Room::RoomPtr room);
    
    // 获取游戏实例
    GamePtr getGame(uint32_t roomId);
    
private:
    GameManager() = default;
    std::map<uint32_t, GamePtr> games_;
};

} // namespace Game
} // namespace Sanguosha

#endif // GAME_GAME_MANAGER_H