#ifndef TFTP_TRANSACTION_HPP
#define TFTP_TRANSACTION_HPP

#include <fstream>

namespace tftp {
struct Transaction {
    enum class State { negotiate,
                       transmite };
    enum class Type { send,
                      receive };

    std::fstream file_;
    State state_;
    Type type_;

    size_t size_;
    size_t last_block_size_;
    uint16_t block_num_;

    uint16_t block_confirmed = 0;
    uint16_t block_sended = 0;
    uint16_t window_size = 1;

    Transaction(std::string filename, Type type, State state, size_t size = 0)
        : type_(type), state_(state) {
        if (type_ == Type::send) {
            file_.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
            size_ = file_.tellg();
            file_.seekg(0, std::ios::beg);
        } else {
            file_.open(filename, std::ios::out | std::ios::binary);
            size_ = size;
        }

        block_num_ = size_ / 512 + 1;
        last_block_size_ = size_ % 512;
    }

    ~Transaction() {
        file_.close();
    }
    
    void confirm_ack(uint16_t block) {
        if (block < block_sended && block >= block_confirmed) {
            state_ = State::transmite;
            block_confirmed  = block + 1;
        }
    }
};
} // namespace tftp

#endif