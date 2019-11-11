#ifndef TFTP_PEER_HPP
#define TFTP_PEER_HPP

#include <algorithm>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <filesystem>
#include <fstream>

#include "TftpMessage.hpp"
#include "TftpParser.hpp"
#include "TftpTransaction.hpp"

using boost::asio::io_context;
using boost::asio::ip::udp;

void dump(const tftp::Buffer &buffer) {
    std::cout << "size: " << buffer.size() << std::endl;

    std::cout << "data: ";
    for (auto &c : buffer) {
        if (isprint(c))
            std::cout << c;
        else
            std::cout << '.';
    }
    std::cout << std::endl;

    std::cout << std::hex;
    std::cout << "hex: ";
    for (auto &c : buffer) {
        std::cout << (int)c << ' ';
    }
    std::cout << std::endl;
    std::cout << std::dec;
};

class TftpPeer {
public:
    TftpPeer(io_context &io_context, unsigned short port)
        : io_context_(io_context),
          socket_cmd_(io_context, udp::v6()),
          socket_data_(io_context, udp::v6()) {

        socket_cmd_.bind(udp::endpoint(udp::v6(), port));
        socket_data_.bind(udp::endpoint(udp::v6(), 0));
        std::cout << "socket cmd bind to " << socket_cmd_.local_endpoint() << std::endl;
        std::cout << "socket data bind to " << socket_data_.local_endpoint() << std::endl;

        start_receive_request();
        start_receive_data();
    }

    void start_write_transaction(std::string filename, udp::endpoint endpoint) {
        std::cout << "send write request " << filename << " " << endpoint << std::endl;

        auto trans = new tftp::SendTransaction(filename);
        pre_send_trans_map_[endpoint.address()] = trans;
        auto size = std::to_string(std::filesystem::file_size(filename));

        auto packet = tftp::WriteRequest::serialize("re_" + filename, tftp::default_mode, {{"tsize", size}});
        socket_data_.send_to(boost::asio::buffer(packet), endpoint);
    }

    void start_read_transaction(std::string filename, udp::endpoint endpoint) {
        std::cout << "send read request " << filename << " " << endpoint << std::endl;

        auto trans = new tftp::RecvTransaction("re_" + filename);
        pre_recv_trans_map_[endpoint.address()] = trans;

        auto packet = tftp::ReadRequest::serialize(filename);
        socket_data_.send_to(boost::asio::buffer(packet), endpoint);
    }

private:
    io_context &io_context_;

    std::map<udp::endpoint, tftp::SendTransaction *> send_trans_map_;
    std::map<udp::endpoint, tftp::RecvTransaction *> recv_trans_map_;
    std::map<boost::asio::ip::address, tftp::SendTransaction *> pre_send_trans_map_;
    std::map<boost::asio::ip::address, tftp::RecvTransaction *> pre_recv_trans_map_;

    udp::socket socket_cmd_;
    udp::endpoint endpoint_cmd_;
    tftp::Buffer buffer_cmd_;

    udp::socket socket_data_;
    udp::endpoint endpoint_data_;
    tftp::Buffer buffer_data_;

    void start_receive_request() {
        buffer_cmd_.resize(1024);

        socket_cmd_.async_receive_from(
            boost::asio::buffer(buffer_cmd_, 1024), endpoint_cmd_,
            [this](boost::system::error_code e, std::size_t bytes_recvd) {
                if (!e && bytes_recvd > 0) {
                    buffer_cmd_.resize(bytes_recvd);
                    tftp::Parser parser(buffer_cmd_);

                    try {
                        if (parser.is_wrq()) {
                            write_request_handle(parser.parser_wrq(), endpoint_cmd_);
                        } else if (parser.is_rrq()) {
                            read_request_handle(parser.parser_rrq(), endpoint_cmd_);
                        } else if (parser.is_error()) {
                        }
                    } catch (std::invalid_argument &e) {
                        std::cout << "wrong format" << std::endl;
                        dump(buffer_cmd_);
                    }
                }
            });
    }

    void start_receive_data() {
        buffer_data_.resize(1024);

        socket_data_.async_receive_from(
            boost::asio::buffer(buffer_data_, 1024), endpoint_data_,
            [this](boost::system::error_code e, std::size_t bytes_recvd) {
                if (!e && bytes_recvd > 0) {
                    buffer_data_.resize(bytes_recvd);
                    tftp::Parser parser(buffer_data_);

                    try {
                        if (parser.is_data()) {
                            data_message_handle(parser.parser_data(), endpoint_data_);
                        } else if (parser.is_ack()) {
                            ack_message_handle(parser.parser_ack(), endpoint_data_);
                        } else if (parser.is_error()) {
                        }
                    } catch (std::invalid_argument &e) {
                        std::cout << "wrong format" << std::endl;
                        dump(buffer_data_);
                    }
                }
            });
    }

    void write_request_handle(tftp::WriteRequest request, udp::endpoint endpoint) {
        // std::cout << "receive: [write request] filename:" << request.filename() << " size:" << request.options().at("tsize") << std::endl;

        if (!recv_trans_map_.count(endpoint) && !pre_recv_trans_map_.count(endpoint.address())) {

            size_t size = std::stoi(request.options().at("tsize"));

            auto trans = new tftp::RecvTransaction(request.filename(), size);
            recv_trans_map_[endpoint] = trans;

            auto packet = tftp::AckMessage::serialize(0);

            // std::cout << "send: [ack] block:" << 0 << std::endl;
            socket_data_.async_send_to(
                boost::asio::buffer(packet), endpoint,
                [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                    if (e) {
                        delete recv_trans_map_[endpoint];
                        recv_trans_map_.erase(endpoint);
                    }
                });
        }

        start_receive_request();
    }

    void read_request_handle(tftp::ReadRequest request, udp::endpoint endpoint) {
        // std::cout << "receive: [read request] filename:" << request.filename() << std::endl;

        if (!send_trans_map_.count(endpoint) && !pre_send_trans_map_.count(endpoint.address())) {
            auto trans = new tftp::SendTransaction(request.filename());
            send_trans_map_[endpoint] = trans;

            auto buffer = trans->get_next_block();
            auto packet = tftp::DataMessage::serialize(0, buffer);

            // std::cout << "send: [data] block:" << 0 << std::endl;
            socket_data_.async_send_to(
                boost::asio::buffer(packet), endpoint,
                [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                    if (e) {
                        delete send_trans_map_[endpoint];
                        send_trans_map_.erase(endpoint);
                    } else {
                        auto trans = send_trans_map_.at(endpoint);
                        trans->confirm_sended();
                    }
                });
        }

        start_receive_request();
    }
    void ack_message_handle(tftp::AckMessage ack, udp::endpoint endpoint) {
        std::cout << "receive: [ack] block:" << ack.block() << std::endl;

        if (ack.block() == 0) {
            if (pre_send_trans_map_.count(endpoint.address())) {
                send_trans_map_[endpoint] = pre_send_trans_map_[endpoint.address()];
                pre_send_trans_map_.erase(endpoint.address());
            }
        }

        if (send_trans_map_.count(endpoint)) {
            auto trans = send_trans_map_.at(endpoint);
            if (trans->confirm_ack(ack.block())) {
                if (trans->is_finished()) {
                    delete send_trans_map_[endpoint];
                    send_trans_map_.erase(endpoint);
                    return;
                }

                auto buffer = trans->get_next_block();
                auto packet = tftp::DataMessage::serialize(ack.block(), buffer);

                std::cout << "send: [data] block:" << ack.block() << std::endl;
                socket_data_.async_send_to(
                    boost::asio::buffer(packet), endpoint,
                    [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                        if (e) {
                            delete send_trans_map_[endpoint];
                            send_trans_map_.erase(endpoint);
                        } else {
                            auto trans = send_trans_map_.at(endpoint);
                            trans->confirm_sended();
                        }
                    });
            }
        }

        start_receive_data();
    }

    void data_message_handle(tftp::DataMessage data, udp::endpoint endpoint) {
        std::cout << "receive: [data] block:" << data.block() << std::endl;

        if (data.block() == 0) {
            if (pre_recv_trans_map_.count(endpoint.address())) {
                recv_trans_map_[endpoint] = pre_recv_trans_map_[endpoint.address()];
                pre_recv_trans_map_.erase(endpoint.address());
            }
        }

        if (recv_trans_map_.count(endpoint)) {
            auto trans = recv_trans_map_.at(endpoint);
            if (trans->receive_data(data)) {
                if (trans->is_finished()) {
                    delete recv_trans_map_[endpoint];
                    recv_trans_map_.erase(endpoint);
                } else {
                    auto packet = tftp::AckMessage::serialize(data.block() + 1);

                    std::cout << "send: [ack] block:" << data.block() + 1 << std::endl;
                    socket_data_.async_send_to(
                        boost::asio::buffer(packet), endpoint,
                        [this, endpoint](boost::system::error_code e, std::size_t bytes_recvd) {
                            if (e) {
                                delete recv_trans_map_[endpoint];
                                recv_trans_map_.erase(endpoint);
                            }
                        });
                }
            }
        }
        start_receive_data();
    }
};

#endif