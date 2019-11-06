#ifndef TFTP_TRANSACTION_HPP
#define TFTP_TRANSACTION_HPP

#include <boost/asio.hpp>
#include <fstream>

#include "TftpMessage.hpp"

using boost::asio::ip::udp;
namespace tftp {
class SendTransaction {
public:
    enum class State { negotiate,
                       transmite,
                       finish };

    std::fstream file_;
    State state_;

    size_t size_;
    size_t last_block_size_;
    uint16_t block_number_;
    uint16_t block_confirmed_ = 0;
    uint16_t block_sended_ = 0;

    SendTransaction(std::string filename) {
        state_ = State::transmite;

        file_.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
        size_ = file_.tellg();
        file_.seekg(0, std::ios::beg);

        block_number_ = size_ / 512 + 1;
        last_block_size_ = size_ % 512;
    }

    ~SendTransaction() {
        file_.close();
    }

    void confirm_ack(uint16_t block) {
        if (block <= block_sended_ && block > block_confirmed_) {
            block_confirmed_ = block;
        }
    }
};

class RecvTransaction {
public:
    enum class State { negotiate,
                       transmite,
                       finish };

    std::fstream file_;
    State state_;

    size_t size_;
    size_t last_block_size_;
    uint16_t block_number_;
    uint16_t block_reveived_;

    RecvTransaction(std::string filename, size_t size) {
        state_ = State::transmite;

        file_.open(filename, std::ios::out | std::ios::binary);
        size_ = size;

        block_number_ = size_ / 512 + 1;
        last_block_size_ = size_ % 512;
    }

    ~RecvTransaction() {
        file_.close();
    }

    void receive_data(tftp::DataMessage &data) {
        if (state_ == State::transmite && data.block() == block_reveived_) {
            file_.write((char *)data.data().data(), data.data().size());
            if (data.data().size() < 512)
                state_ = State::finish;
            block_reveived_ += 1;
        }
    }
};
}  // namespace tftp

#endif