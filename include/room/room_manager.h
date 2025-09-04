#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_context.hpp>
#include "room/room.h" // 包含完整定义

namespace Sanguosha {
namespace Room {

class RoomManager {
public:
    static RoomManager& Instance();
    
    uint32_t createRoom();
    uint32_t createRoom(const std::vector<uint32_t>& playerIds);
    bool joinRoom(uint32_t roomId, uint32_t playerId);
    bool leaveRoom(uint32_t roomId, uint32_t playerId);
    std::shared_ptr<Room> getRoom(uint32_t roomId); // 保持原始返回类型
    void setIoContext(boost::asio::io_context& io);
    void startCleanupTask();
    uint32_t matchPlayers(const std::vector<uint32_t>& playerIds); // 添加声明
    
private:
    RoomManager();
    ~RoomManager();
    void cleanupRooms();
    
    std::unordered_map<uint32_t, std::shared_ptr<Room>> rooms_;
    uint32_t nextRoomId_ = 1;
    std::mutex mutex_;
    boost::asio::io_context* io_ = nullptr;
    std::unique_ptr<boost::asio::steady_timer> cleanupTimer_;
    
    static boost::asio::io_context dummy_io_context_;
};

} // namespace Room
} // namespace Sanguosha