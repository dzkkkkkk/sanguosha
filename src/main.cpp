#include <iostream>
#include "network/server.h"
#include "room/room_manager.h"

int main() {
    std::cout << "Starting Sanguosha Server..." << std::endl;
    
    try {
        Sanguosha::Network::Server server;
        
        // 初始化房间管理器
        auto& roomMgr = Sanguosha::Room::RoomManager::Instance();
        roomMgr.setIoContext(server.getIoContext());
        
        // 启动房间清理任务
        roomMgr.startCleanupTask();
        
        server.start(9527);
    } catch (const std::exception& e) {
        std::cerr << "Server fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}