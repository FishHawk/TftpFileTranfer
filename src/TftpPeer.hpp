#ifndef TFTP_PEER_HPP
#define TFTP_PEER_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include "Tftp.hpp"
#include "TftpParser.hpp"

using boost::asio::ip::udp;
using boost::asio::local::datagram_protocol;

class TftpPeer {
public:
    static constexpr unsigned short default_port = 10000;

    TftpPeer(boost::asio::io_context &io_context, unsigned short port)
        : socket_data_(io_context, udp::endpoint(udp::v6(), port)) {
        std::cout << "bind to port " << port << std::endl;

        // for test
        if (port == 10000)
            start_transaction("test.jpg", "::1", 10001);
        else
            start_receive();

        // tftp::PacketRrq d("dddd");
        // d.dump();
        // tftp::Parser pa(d.data);
        // try {
        //     auto n_rrq = pa.parser_rrq();
        //     n_rrq.dump();
        // } catch (std::invalid_argument &e) {
        //     std::cout << "error" << std::endl;
        // }
    }

private:
    udp::socket socket_data_;
    // udp::endpoint remote_endpoint_;
    // boost::array<char, 1> recv_buffer_;

    void start_transaction(std::string filename, std::string dst_ip, unsigned short dst_port) {
        std::cout << "send write request" << std::endl;

        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::cout << size << std::endl;

        tftp::PacketWrq wrq(filename, tftp::default_mode, {{"tsize", std::to_string(size)}});
        wrq.dump();

        udp::endpoint receiver_endpoint(boost::asio::ip::make_address_v6(dst_ip), dst_port);
        std::cout << "dst:" << receiver_endpoint << std::endl;

        socket_data_.send_to(boost::asio::buffer(wrq.data), receiver_endpoint);
        socket_data_.send_to(boost::asio::buffer("123456"), receiver_endpoint);

        // boost::array<char, 128> recv_buf;
        // udp::endpoint sender_endpoint;
        // size_t len = socket.receive_from(
        //     boost::asio::buffer(recv_buf), sender_endpoint);

        // std::cout.write(recv_buf.data(), len);
    }

    void start_receive() {
        std::vector<uint8_t> raw_packet;
        raw_packet.resize(1024);

        udp::endpoint sender_endpoint;
        size_t len = socket_data_.receive_from(boost::asio::buffer(raw_packet), sender_endpoint);
        
        // tftp::Parser parser(raw_packet);

        std::cout.write((char *)raw_packet.data(), len);
    }
};

#endif