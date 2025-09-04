#include "network/server.h"
#include "network/session.h"

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
                auto session = std::make_shared<Session>(std::move(socket));
                sessions_.insert(session);
                session->start();
            }
            do_accept();
        });
}

} // namespace Network
} // namespace Sanguosha