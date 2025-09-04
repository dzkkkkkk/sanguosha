#ifndef GAME_GAME_INSTANCE_H
#define GAME_GAME_INSTANCE_H

#include <vector>
#include <cstdint>
#include "room/room.h"

namespace Sanguosha {
namespace Game {

class GameInstance {
public:
    explicit GameInstance(Sanguosha::Room::RoomPtr room);
    
    // 游戏流程控制
    void startChoosing();  // 开始选将阶段
    void startGame();      // 开始游戏阶段
    void processTurn();    // 处理回合
    void endGame();        // 结束游戏
    
private:
    Sanguosha::Room::RoomPtr room_;
    // TODO: 添加游戏状态数据
};

} // namespace Game
} // namespace Sanguosha

#endif // GAME_GAME_INSTANCE_H