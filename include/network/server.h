#pragma once
#include <boost/asio.hpp>
#include <set>
#include <unordered_map>
#include <mutex>
#include <memory>
#include "network/session.h"

namespace Sanguosha {
namespace Network {

class Session; // 前向声明

class Server {
public:
    Server();
    void start(unsigned short port);
    
    // 添加三个关键的会话管理方法
    void registerSession(uint32_t playerId, std::shared_ptr<Session> session);
    void unregisterSession(uint32_t playerId);
    std::shared_ptr<Session> getSession(uint32_t playerId);
    
    // 添加获取io_context的方法
    boost::asio::io_context& getIoContext() { return io_context_; }

private:
    void do_accept();
    
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    // 用于管理所有活跃会话的集合
    std::set<std::shared_ptr<Session>> sessions_;
    
    // 用于通过玩家ID查找其会话的映射表
    std::unordered_map<uint32_t, std::weak_ptr<Session>> playerSessions_;
    std::mutex sessionMutex_; // 保护 playerSessions_ 的互斥锁
};

} // namespace Network
} // namespace Sanguosha