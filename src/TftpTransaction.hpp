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
        size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);

        block_number_ = size_ / block_size_ + 1;
        last_block_size_ = size_ % block_size_;
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
            buffer.resize(block_size_);
            file_.read((char *)buffer.data(), block_size_);
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
        return block_size_ * speed_monitor.speed();
    }

    bool set_option_blksize(uint16_t blksize) {
        if (blksize <= tftp::max_block_size && blksize >= tftp::min_block_size) {
            has_blksize_option_ = true;
            block_size_ = blksize;

            block_number_ = size_ / block_size_ + 1;
            last_block_size_ = size_ % block_size_;
            return true;
        } else {
            return false;
        }
    }

private:
    std::string filename_;
    std::fstream file_;

    bool is_finished_ = false;

    size_t size_;
    size_t last_block_size_;
    uint16_t block_number_;
    uint16_t block_sended_ = 0;

    SpeedMonitor speed_monitor;

    // for option "blksize"
    bool has_blksize_option_ = false;
    uint16_t block_size_ = tftp::block_size;
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
        if (block != block_received_) {
            return false;
        } else {
            file_.write((char *)data.data().data(), data.data().size());
            std::cout<<data.data().size()<<std::endl;
            if (data.data().size() < block_size_) {
                is_finished_ = true;
            }

            block_received_ += 1;
            return true;
        }
    }

    // return send speed, unit bytes/s
    double speed() {
        return block_size_ * speed_monitor.speed();
    }

    bool set_option_tsize(size_t size) {
        has_size_option_ = true;
        size_ = size;
        return true;
    }

    bool set_option_blksize(uint16_t blksize) {
        if (blksize <= tftp::max_block_size && blksize >= tftp::min_block_size) {
            has_blksize_option_ = true;
            block_size_ = blksize;
            return true;
        } else {
            return false;
        }
    }

private:
    std::string filename_;
    std::fstream file_;

    bool is_finished_ = false;
    uint16_t block_received_ = 0;

    SpeedMonitor speed_monitor;

    // for option "tsize"
    bool has_size_option_ = false;
    size_t size_ = 0;

    // for option "blksize"
    bool has_blksize_option_ = false;
    uint16_t block_size_ = tftp::block_size;
};
}  // namespace tftp

#endif