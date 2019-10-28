#ifndef TFTP_PEER_HPP
#define TFTP_PEER_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Tftp.hpp"
#include "TftpParser.hpp"

using boost::asio::ip::udp;
using boost::asio::local::datagram_protocol;

class TftpPeer {
public:
    static constexpr int default_port = 10000;

    TftpPeer(boost::asio::io_context &io_context, int port)
        : socket_data_(io_context, udp::endpoint(udp::v6(), port)) {
        std::cout << "bind to port " << port << std::endl;

        tftp::TftpRrqPacket p("2213123");
        p.dump();

        tftp::Packet pp(p.get_data());
        pp.dump();

        auto ppp = tftp::parsing_rrq(pp);
        if (ppp) {
            ppp->dump();
            std::cout << ppp->get_filename() << std::endl;
            std::cout << ppp->get_mode() << std::endl;
        } else
            std::cout << "wrong" << std::endl;
    }

private:
    udp::socket socket_data_;
    // udp::endpoint remote_endpoint_;
    // boost::array<char, 1> recv_buffer_;

    void start_transmission(std::string filename, std::string dst_ip, int dst_port) {
    }

    void start_receive() {
        // socket_data_.async_receive_from(
        //     boost::asio::buffer(recv_buffer_), remote_endpoint_,
        //     boost::bind(&TftpPeer::handle_receive, this,
        //                 boost::asio::placeholders::error,
        //                 boost::asio::placeholders::bytes_transferred));
    }
};

#endif