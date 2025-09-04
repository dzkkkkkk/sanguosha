#ifndef ROOM_ROOM_MANAGER_H
#define ROOM_ROOM_MANAGER_H

#include <map>
#include <vector>
#include <memory>
#include <mutex>

namespace Sanguosha {
namespace Room {

class Room;
using RoomPtr = std::shared_ptr<Room>;

class RoomManager {
public:
    static RoomManager& Instance();
    
    // 创建房间（返回房间ID）
    uint32_t createRoom();
    
    // 加入房间
    bool joinRoom(uint32_t roomId, uint32_t playerId);
    
    // 离开房间
    bool leaveRoom(uint32_t roomId, uint32_t playerId);
    
    // 获取房间
    RoomPtr getRoom(uint32_t roomId);
    
private:
    RoomManager() = default;
    std::map<uint32_t, RoomPtr> rooms_;
    std::mutex mutex_;
    uint32_t nextRoomId_ = 1000; // 起始房间ID
};

} // namespace Room
} // namespace Sanguosha

#endif // ROOM_ROOM_MANAGER_H