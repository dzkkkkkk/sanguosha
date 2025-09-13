#pragma once

#include <memory>
#include <string>
#include <functional>
#include "proto/generated/sanguosha.pb.h"

namespace sanguosha {

class Connection;

class GameClient {
public:
    GameClient();
    ~GameClient();
    
    // 连接服务器
    bool connect(const std::string& host, uint16_t port);
    
    // 登录
    bool login(const std::string& username, const std::string& password);
    
    // 房间操作
    bool createRoom();
    bool joinRoom(uint32_t roomId);
    bool leaveRoom();
    
    // 游戏操作
    bool playCard(uint32_t cardId, uint32_t targetPlayer = 0);
    bool endTurn();
    
    // 设置回调函数
    void setGameStateCallback(std::function<void(const GameState&)> callback);
    void setGameStartCallback(std::function<void(const GameStart&)> callback);
    
    // 运行客户端主循环
    void run();
    void stop();
    
private:
    void handleIncomingMessages();
    void sendMessage(const GameMessage& message);
    
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}