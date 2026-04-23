#include "network/server.h"
#include "room/room_manager.h"
#include "database/database_manager.h"

int main() {
    std::cout << "Starting Sanguosha Server v1.0 with Database Support" << std::endl;
    
    try {
        // 初始化数据库
        if (!Sanguosha::Database::DatabaseManager::Instance().initialize()) {
            std::cerr << "Failed to initialize database" << std::endl;
            return 1;
        }
        
        Sanguosha::Network::Server server;
        
        // 关键：将Server实例设置给RoomManager单例
        Sanguosha::Room::RoomManager::Instance().setServer(server);
        // 如果需要，还可以设置IOContext（如果你实现了清理任务）
        // Sanguosha::Room::RoomManager::Instance().setIoContext(server.getIoContext());
        // Sanguosha::Room::RoomManager::Instance().startCleanupTask();
        
        server.start(9527);
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }    
    return 0;
}