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

    std::map<udp::endpoint, tftp::SendTransaction *> send_trans_map_;
    std::map<udp::endpoint, tftp::RecvTransaction *> recv_trans_map_;

    void start_transaction(std::string filename, std::string dst_ip, unsigned short dst_port) {
        std::cout << "send write request" << std::endl;

        udp::endpoint endpoint(boost::asio::ip::make_address_v6(dst_ip), dst_port);
        auto trans = new tftp::SendTransaction(filename);
        send_trans_map_[endpoint] = trans;
        auto size = std::to_string(trans->size_);

        auto packet = tftp::WriteRequest::serialize("re_" + filename, tftp::default_mode, {{"tsize", size}});
        socket_data_.send_to(boost::asio::buffer(packet.raw_), endpoint);
    }

    void start_receive() {
        // FIXME: direct call
        recv_buffer_.raw_.resize(1024);

        socket_data_.async_receive_from(
            boost::asio::buffer(recv_buffer_.raw_, 1024), remote_endpoint_,
            [this](boost::system::error_code e, std::size_t bytes_recvd) {
                if (!e && bytes_recvd > 0) {
                    // FIXME: direct call
                    recv_buffer_.raw_.resize(bytes_recvd);
                    tftp::Parser parser(recv_buffer_.raw_);
                    // recv_buffer_.dump();

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
                        recv_buffer_.dump();
                    }
                }
            });
    }

    void write_request_handle(tftp::WriteRequest &request, udp::endpoint endpoint) {
        std::cout << "receive: [write request] filename:" << request.filename() << " size:" << request.options().at("tsize") << std::endl;

        if (!recv_trans_map_.count(endpoint)) {
            size_t size = std::stoi(request.options().at("tsize"));

            auto trans = new tftp::RecvTransaction(request.filename(), size);
            recv_trans_map_[endpoint] = trans;

            auto packet = tftp::Ack::serialize(0);

            std::cout << "send: [ack] block:" << 0 << std::endl;
            socket_data_.async_send_to(
                boost::asio::buffer(packet.raw_), endpoint,
                [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                    if (e) {
                        delete recv_trans_map_[endpoint];
                        recv_trans_map_.erase(endpoint);
                    }
                });
        }

        start_receive();
    }

    void ack_handle(tftp::Ack &ack, udp::endpoint endpoint) {
        std::cout << "receive: [ack] block:" << ack.block() << std::endl;

        if (send_trans_map_.count(endpoint)) {
            auto trans = send_trans_map_.at(endpoint);
            trans->confirm_ack(ack.block());

            if (trans->block_confirmed_ == trans->block_sended_) {
                auto block = trans->block_sended_;

                std::vector<uint8_t> buffer;
                if (block < trans->block_number_) {
                    buffer.resize(512);
                    trans->file_.read((char *)buffer.data(), 512);
                } else {
                    buffer.resize(trans->last_block_size_);
                    trans->file_.read((char *)buffer.data(), trans->last_block_size_);
                }
                std::cout << "!!!!!" << buffer.size() << std::endl;

                auto packet = tftp::Data::serialize(block, buffer);

                std::cout << "send: [data] block:" << block << std::endl;
                socket_data_.async_send_to(
                    boost::asio::buffer(packet.raw_), endpoint,
                    [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                        if (e) {
                            delete send_trans_map_[endpoint];
                            send_trans_map_.erase(endpoint);
                        }
                    });

                trans->block_sended_ += 1;
            }
        }

        start_receive();
    }

    void data_handle(tftp::Data &data, udp::endpoint endpoint) {
        std::cout << "receive: [data] block:" << data.block() << std::endl;

        if (recv_trans_map_.count(endpoint)) {
            auto trans = recv_trans_map_.at(endpoint);
            trans->receive_data(data);

            auto block = trans->block_reveived_;
            auto packet = tftp::Ack::serialize(block);

            std::cout << "send: [ack] block:" << block << std::endl;
            socket_data_.async_send_to(
                boost::asio::buffer(packet.raw_), endpoint,
                [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                    if (e) {
                        delete recv_trans_map_[endpoint];
                        recv_trans_map_.erase(endpoint);
                    }
                });
        }
        start_receive();
    }
};

#endif