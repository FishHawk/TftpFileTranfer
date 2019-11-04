#ifndef TFTP_PEER_HPP
#define TFTP_PEER_HPP

#include <algorithm>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <fstream>

#include "Tftp.hpp"
#include "TftpParser.hpp"
#include "TftpTransaction.hpp"

using boost::asio::ip::udp;
using boost::asio::local::datagram_protocol;

class TftpPeer {
public:
    static constexpr unsigned short default_port = 10000;

    TftpPeer(boost::asio::io_context &io_context, unsigned short port)
        : socket_data_(io_context, udp::endpoint(udp::v6(), port)) {
        std::cout << "bind to port " << port << std::endl;

        start_receive();

        // FIXME: for test
        if (port == 10000)
            start_transaction("test.jpg", "::1", 10001);
    }

private:
    udp::socket socket_data_;
    udp::endpoint remote_endpoint_;
    tftp::Packet recv_buffer_;

    std::map<udp::endpoint, tftp::Transaction> transactions_;

    void start_transaction(std::string filename, std::string dst_ip, unsigned short dst_port) {
        std::cout << "send write request" << std::endl;

        udp::endpoint endpoint(boost::asio::ip::make_address_v6(dst_ip), dst_port);
        transactions_[endpoint] = tftp::Transaction();
        tftp::Transaction &transaction = transactions_[endpoint];

        transaction.file.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
        int size = transaction.file.tellg();
        transaction.file.seekg(0, std::ios::beg);
        transaction.type = tftp::Transaction::Type::send;
        transaction.state = tftp::Transaction::State::negotiate;

        auto packet = tftp::WriteRequest::serialize("re_" + filename, tftp::default_mode, {{"tsize", std::to_string(size)}});
        socket_data_.send_to(boost::asio::buffer(packet.raw_), endpoint);
    }

    void start_receive() {
        // FIXME: direct call
        recv_buffer_.raw_.resize(512);

        socket_data_.async_receive_from(
            boost::asio::buffer(recv_buffer_.raw_, 512), remote_endpoint_,
            [this](boost::system::error_code e, std::size_t bytes_recvd) {
                if (!e && bytes_recvd > 0) {
                    // FIXME: direct call
                    recv_buffer_.raw_.resize(bytes_recvd);
                    recv_buffer_.dump();
                    tftp::Parser parser(recv_buffer_.raw_);
                    std::cout << parser.is_wrq() << std::endl;

                    try {
                        if (parser.is_wrq()) {
                            auto wrq = parser.parser_wrq();
                            write_request_handle(wrq, remote_endpoint_);
                        } else if (parser.is_rrq()) {
                        } else if (parser.is_data()) {
                            auto data = parser.parser_data();
                            data_handle(data, remote_endpoint_);
                        } else if (parser.is_ack()) {
                            auto ack = parser.parser_ack();
                            ack_handle(ack, remote_endpoint_);
                        } else if (parser.is_error()) {
                        }
                    } catch (std::invalid_argument &e) {
                        std::cout << "wrong format" << std::endl;
                    }
                }
            });
    }

    void write_request_handle(tftp::WriteRequest &request, udp::endpoint endpoint) {
        if (!transactions_.count(endpoint)) {
            transactions_[endpoint] = tftp::Transaction();
            tftp::Transaction &transaction = transactions_[endpoint];

            transaction.file.open(request.filename(), std::ios::out | std::ios::binary);
            transaction.type = tftp::Transaction::Type::receive;
            transaction.state = tftp::Transaction::State::transmite;

            auto packet = tftp::Ack::serialize(0);
            socket_data_.async_send_to(
                boost::asio::buffer(packet.raw_), endpoint,
                [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                    if (e) {
                        transactions_[endpoint].file.close();
                        transactions_.erase(endpoint);
                    }
                });
        }

        start_receive();
    }

    void ack_handle(tftp::Ack &ack, udp::endpoint endpoint) {
        if (transactions_.count(endpoint)) {
            tftp::Transaction &transaction = transactions_[endpoint];
            transaction.state = tftp::Transaction::State::transmite;
            transaction.block_comfirmed = std::max(ack.block(), transaction.block_comfirmed);

            std::vector<uint8_t> buffer(512);
            transaction.file.read((char *)buffer.data(), 512);
            if (transaction.block_total < transaction.block_comfirmed + transaction.window_size) {
                auto block = transaction.block_total;
                auto packet = tftp::Data::serialize(block, buffer);
                socket_data_.async_send_to(
                    boost::asio::buffer(packet.raw_), endpoint,
                    [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                        if (e) {
                            transactions_[endpoint].file.close();
                            transactions_.erase(endpoint);
                        }
                    });
            }
        }

        start_receive();
    }

    void data_handle(tftp::Data &data, udp::endpoint endpoint) {
        std::cout << "yes" << std::endl;
        std::cout << data.block() << std::endl;
        start_receive();
    }
};

#endif