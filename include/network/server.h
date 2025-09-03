#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <boost/asio.hpp>

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
};

} // namespace Network
} // namespace Sanguosha

#endif // NETWORK_SERVER_H