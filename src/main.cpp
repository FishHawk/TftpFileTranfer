#include <iostream>

#include <boost/asio.hpp>

#include "TftpPeer.hpp"

int main(int argc, char *argv[]) {
    int port = TftpPeer::default_port;
    if (argc > 1) port = std::stoi(argv[1]);

    try {
        boost::asio::io_context io_context;
        TftpPeer peer(io_context, port);
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}