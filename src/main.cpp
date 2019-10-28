#include <iostream>

#include <boost/asio.hpp>

#include "TftpPeer.hpp"

int main(int argc, char *argv[]) {
    try {
        boost::asio::io_context io_context;
        TftpPeer peer(io_context, 10000);
        io_context.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}