#pragma once

#include <vector>
#include <cstdint>
#include <mutex>
#include <memory>

// 前向声明
namespace Sanguosha {
namespace Room {
    class RoomManager;
}
namespace Network {
    class Server; // 添加Server的前向声明
}
}

namespace sanguosha {
    class GameInstance;
}

namespace Sanguosha {
namespace Room {

class Room {
public:
    enum class State { WAITING, PLAYING };
    
    explicit Room(uint32_t id);
    
    bool addPlayer(uint32_t playerId);
    bool removePlayer(uint32_t playerId);
    
    bool startGame(RoomManager& roomManager, Sanguosha::Network::Server& server); // 使用完整命名空间
    
    uint32_t playerCount() const;
    uint32_t id() const;
    State state() const;
    
    const std::vector<uint32_t>& getPlayers() const;
    
    bool isPlaying() const;
    std::shared_ptr<sanguosha::GameInstance> getGameInstance() const;

private:
    uint32_t id_;
    std::vector<uint32_t> players_;
    State state_;
    std::mutex mutex_;
    
    std::shared_ptr<sanguosha::GameInstance> gameInstance_;
};

} // namespace Room
} // namespace Sanguosha