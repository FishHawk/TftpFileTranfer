#ifndef TFTP_TRANSACTION_HPP
#define TFTP_TRANSACTION_HPP

#include <fstream>

namespace tftp {
struct Transaction {
    enum class State { negotiate,
                       transmite };
    enum class Type { send,
                      receive };

    std::fstream file;
    State state;
    Type type;

    uint16_t block_comfirmed = 0;
    uint16_t block_total = 0;
    uint16_t window_size = 1;
};
} // namespace tftp

#endif