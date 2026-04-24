#include <iostream>
#include <string>
#include "../include/database/database_manager.h"

int main() {
    std::cout << "Starting database test..." << std::endl;

    // 初始化数据库
    std::cout << "Initializing database..." << std::endl;
    if (!Sanguosha::Database::DatabaseManager::Instance().initialize("test.db")) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    std::cout << "Database initialized successfully" << std::endl;

    // 测试注册功能
    std::cout << "Testing registration..." << std::endl;
    uint32_t userId;
    if (Sanguosha::Database::DatabaseManager::Instance().registerUser("testuser", "testpass", "test@example.com", userId)) {
        std::cout << "Registration successful! User ID: " << userId << std::endl;
    } else {
        std::cout << "Registration failed!" << std::endl;
    }

    // 测试登录功能
    std::cout << "Testing login..." << std::endl;
    if (Sanguosha::Database::DatabaseManager::Instance().authenticateUser("testuser", "testpass", userId)) {
        std::cout << "Login successful! User ID: " << userId << std::endl;
    } else {
        std::cout << "Login failed!" << std::endl;
    }

    std::cout << "Test completed." << std::endl;
    return 0;
}