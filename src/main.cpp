#include <boost/asio.hpp>
#include <iostream>

#include "TftpPeer.hpp"

int main(int argc, char *argv[]) {
    unsigned short port = tftp::default_port;
    if (argc > 1)
        port = std::stoi(argv[1]);

    try {
        boost::asio::io_context io_context;
        TftpPeer peer(io_context, port);

        std::thread t([&io_context]() { io_context.run(); });

        std::string line;
        while (std::getline(std::cin, line)) {
            std::istringstream cmd(line);

            std::string op;
            cmd >> op;

            if (op == "send") {
                std::string filename, dst_ip;
                uint16_t dst_port;
                cmd >> filename >> dst_ip >> dst_port;
                boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::make_address_v6(dst_ip), dst_port);

                peer.start_write_transaction(filename, endpoint);
            } else if (op == "get") {
                std::string filename, dst_ip;
                uint16_t dst_port;
                cmd >> filename >> dst_ip >> dst_port;
                boost::asio::ip::udp::endpoint endpoint(boost::asio::ip::make_address_v6(dst_ip), dst_port);

                peer.start_read_transaction(filename, endpoint);
            } else {
                std::cout << "wrong format" << std::endl;
            }
        }

        t.join();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}