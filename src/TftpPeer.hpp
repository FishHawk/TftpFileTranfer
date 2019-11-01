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
        recv_buffer_.raw_.resize(512);
        std::cout << "bind to port " << port << std::endl;

        start_receive();
        // for test
        if (port == 10000)
            start_transaction("test.jpg", "::1", 10001);
    }

private:
    udp::socket socket_data_;
    udp::endpoint remote_endpoint_;
    tftp::Packet recv_buffer_;

    void start_transaction(std::string filename, std::string dst_ip, unsigned short dst_port) {
        std::cout << "send write request" << std::endl;

        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        int size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::cout << size << std::endl;

        auto sender_buffer = tftp::WriteRequest::serialize(filename, tftp::default_mode, {{"tsize", std::to_string(size)}});
        sender_buffer->dump();

        udp::endpoint receiver_endpoint(boost::asio::ip::make_address_v6(dst_ip), dst_port);
        std::cout << "dst:" << receiver_endpoint << std::endl;

        socket_data_.send_to(boost::asio::buffer(sender_buffer->raw_), receiver_endpoint);
    }

    void start_receive() {
        socket_data_.async_receive_from(
            boost::asio::buffer(recv_buffer_.data(), 512), remote_endpoint_,
            [this](boost::system::error_code e, std::size_t bytes_recvd) {
                if (!e && bytes_recvd > 0) {
                    recv_buffer_.raw_.resize(bytes_recvd);
                    recv_buffer_.dump();
                    tftp::Parser parser(recv_buffer_.raw_);
                    std::cout << parser.is_wrq()<<std::endl;

                    try {
                        if (parser.is_wrq()) {
                            auto wrq = parser.parser_wrq();
                            wrq_handle(wrq, remote_endpoint_);
                        } else if (parser.is_rrq())
                            ;
                        else if (parser.is_data())
                            ;
                        else if (parser.is_ack())
                            ;
                        else if (parser.is_error())
                            ;
                    } catch (std::invalid_argument &e) {
                        std::cout << "wrong format" << std::endl;
                    }
                    recv_buffer_.raw_.resize(512);
                }
            });
    }

    void wrq_handle(tftp::WriteRequest &wrq, udp::endpoint endpoint) {
        std::cout << wrq.filename() << std::endl;
    }
};

#endif