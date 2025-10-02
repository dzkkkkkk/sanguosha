#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "sanguosha.pb.h"

// 修改前向声明
namespace Sanguosha {
namespace Network {
class Server; // 保持前向声明
}
}

namespace Sanguosha {
namespace Network {

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::ip::tcp::socket socket, Server& server);
    ~Session(); // 添加析构函数声明
    
    void start();
    void send(const sanguosha::GameMessage& msg);
    
private:
    void doReadHeader();
    void doReadBody();
    void handleLogin(const sanguosha::LoginRequest& login);
    void handleHeartbeat(const boost::system::error_code& ec);
    void startHeartbeat();
    void handleRoomRequest(const sanguosha::RoomRequest& request);
    void handleRoomListRequest();
    void handleGameAction(const sanguosha::GameAction& action);
    
    boost::asio::ip::tcp::socket socket_;
    boost::asio::steady_timer heartbeat_timer_;
    std::array<char, 128> header_buffer_;
    std::vector<char> body_buffer_;
    uint32_t expected_body_size_ = 0;
    uint32_t playerId_ = 0;
    static constexpr int HEARTBEAT_INTERVAL = 30;
    static constexpr int HEARTBEAT_TIMEOUT = 60;

    Server& server_; // 使用前向声明的Server类型
};

} // namespace Network
} // namespace Sanguosha