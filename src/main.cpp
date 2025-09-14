#include <iostream>
#include "network/server.h"

int main() {
    std::cout << "Starting Simplified Sanguosha Server v1.0" << std::endl;
    
    try {
        Sanguosha::Network::Server server;
        server.start(9527); // 使用默认端口
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}