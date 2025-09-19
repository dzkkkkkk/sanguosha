#include "network/server.h"
#include "network/session.h"
#include <iostream>

using boost::asio::ip::tcp;

namespace Sanguosha {
namespace Network {

Server::Server() 
    : io_context_(),
      acceptor_(io_context_) {}

void Server::start(unsigned short port) {
    tcp::endpoint endpoint(tcp::v4(), port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    
    do_accept();
    
    std::cout << "Server listening on port " << port << std::endl;
    io_context_.run();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "New connection accepted" << std::endl;
                // 创建Session时，传入this（Server）的引用
                auto session = std::make_shared<Session>(std::move(socket), *this);
                sessions_.insert(session);
                session->start();
            }
            do_accept();
        });
}

void Server::registerSession(uint32_t playerId, std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    playerSessions_[playerId] = session; // 存储shared_ptr，避免循环引用
    std::cout << "Player " << playerId << " session registered." << std::endl;
}

void Server::unregisterSession(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    playerSessions_.erase(playerId);
    std::cout << "Player " << playerId << " session unregistered." << std::endl;
}

std::shared_ptr<Session> Server::getSession(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(sessionMutex_);
    auto it = playerSessions_.find(playerId);
    if (it != playerSessions_.end()) {
        return it->second.lock(); // 使用lock()将weak_ptr转为shared_ptr
    }
    return nullptr;
}

} // namespace Network
} // namespace Sanguosha