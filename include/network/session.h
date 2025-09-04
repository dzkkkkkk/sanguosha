#ifndef NETWORK_SESSION_H
#define NETWORK_SESSION_H

#include <boost/asio.hpp>
#include <memory>
#include "sanguosha.pb.h"

namespace Sanguosha {
namespace Network {

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::ip::tcp::socket socket);
    void start();
    void send(const sanguosha::GameMessage& msg);
    
private:
    void doReadHeader();
    void doReadBody();
    void handleLogin(const sanguosha::LoginRequest& login);
    void handleHeartbeat(const boost::system::error_code& ec);
    void startHeartbeat();
    void handleRoomRequest(const sanguosha::RoomRequest& request);
    
    boost::asio::ip::tcp::socket socket_;
    boost::asio::steady_timer heartbeat_timer_;
    std::array<char, 128> header_buffer_;
    std::vector<char> body_buffer_;
    uint32_t expected_body_size_ = 0;
    uint32_t playerId_ = 0; // 添加玩家ID成员
    static constexpr int HEARTBEAT_INTERVAL = 30;
    static constexpr int HEARTBEAT_TIMEOUT = 60;
};

} // namespace Network
} // namespace Sanguosha

#endif // NETWORK_SESSION_H