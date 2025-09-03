#include <iostream>
#include "network/server.h"

int main() {
    std::cout << "Starting Sanguosha Server..." << std::endl;
    
    try {
        Sanguosha::Network::Server server;
        server.start(9527);  // 使用三国杀默认端口
    } catch (const std::exception& e) {
        std::cerr << "Server fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}