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
};
} // namespace tftp

#endif