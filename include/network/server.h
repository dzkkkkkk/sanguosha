#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <boost/asio.hpp>
#include <set>
#include "network/session.h"

namespace Sanguosha {
namespace Network {

class Server {
public:
    Server();
    void start(unsigned short port);
    
private:
    void do_accept();
    
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::set<std::shared_ptr<Session>> sessions_;
};

} // namespace Network
} // namespace Sanguosha

#endif // NETWORK_SERVER_H