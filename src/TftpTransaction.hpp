#ifndef TFTP_TRANSACTION_HPP
#define TFTP_TRANSACTION_HPP

#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
#include <iostream>

#include "SpeedMonitor.hpp"
#include "TftpMessage.hpp"

using boost::asio::ip::udp;
namespace tftp {

class SendTransaction {
public:
    SendTransaction(std::string filename) {
        filename_ = filename;
        file_.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
        auto size = file_.tellg();
        file_.seekg(0, std::ios::beg);

        block_number_ = size / tftp::block_size + 1;
        last_block_size_ = size % tftp::block_size;
    }

    ~SendTransaction() {
        file_.close();
    }

    bool is_finished() {
        return is_finished_;
    }

    bool confirm_ack(uint16_t block) {
        if (block_number_ == block_sended_ && block == block_sended_) {
            is_finished_ = true;
        }
        return block == block_sended_;
    }

    Buffer get_next_block() {
        Buffer buffer;
        if (block_sended_ < block_number_ - 1) {
            buffer.resize(512);
            file_.read((char *)buffer.data(), 512);
        } else {
            buffer.resize(last_block_size_);
            file_.read((char *)buffer.data(), last_block_size_);
        }
        return buffer;
    }

    void confirm_sended() {
        speed_monitor.tick();
        block_sended_ += 1;
    }

    // return send speed, unit bytes/s
    double speed() {
        return tftp::block_size * speed_monitor.speed();
    }

private:
    std::string filename_;
    std::fstream file_;

    bool is_finished_ = false;

    size_t last_block_size_;
    uint16_t block_number_;
    uint16_t block_sended_ = 0;

    SpeedMonitor speed_monitor;
};

class RecvTransaction {
public:
    RecvTransaction(std::string filename) {
        filename_ = filename;
        file_.open(filename, std::ios::out | std::ios::binary);
    }

    RecvTransaction(std::string filename, size_t size) {
        filename_ = filename;
        file_.open(filename, std::ios::out | std::ios::binary);

        has_size_option_ = true;
        size_ = size;
    }

    ~RecvTransaction() {
        file_.close();
    }

    bool is_finished() {
        return is_finished_;
    }

    bool receive_data(tftp::DataMessage &data) {
        speed_monitor.tick();

        auto block = data.block();
        if (block != block_reveived_) {
            return false;
        } else {
            file_.write((char *)data.data().data(), data.data().size());
            if (data.data().size() < tftp::block_size) {
                is_finished_ = true;
            }

            block_reveived_ += 1;
            return true;
        }
    }

    // return send speed, unit bytes/s
    double speed() {
        return tftp::block_size * speed_monitor.speed();
    }

private:
    std::string filename_;
    std::fstream file_;

    bool is_finished_ = false;
    uint16_t block_reveived_ = 0;

    SpeedMonitor speed_monitor;

    // for option "tsize"
    bool has_size_option_ = false;
    size_t size_ = 0;
};
} // namespace tftp

#endif