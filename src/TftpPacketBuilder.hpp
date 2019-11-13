#ifndef TFTP_PACKET_BUILDER_HPP
#define TFTP_PACKET_BUILDER_HPP

#include <cstdint>
#include <vector>
#include <map>
#include <string>

namespace tftp {
static const uint16_t default_port = 10000;
static const size_t block_size = 512;

static const uint16_t opcode_rrq = 1;
static const uint16_t opcode_wrq = 2;
static const uint16_t opcode_data = 3;
static const uint16_t opcode_ack = 4;
static const uint16_t opcode_error = 5;
static const uint16_t opcode_oack = 6;

enum class Mode { netascii,
                  octet,
                  mail };
static const Mode default_mode = Mode::octet;

static const std::map<Mode, const char *> mode_to_string = {
    {Mode::netascii, "netascii"},
    {Mode::octet, "octet"},
    {Mode::mail, "mail"},
};
} // namespace tftp

namespace tftp {
using Buffer = std::vector<uint8_t>;

class PacketBuilder {
private:
    friend class Parser;

    std::vector<uint8_t> packet_;

public:
    const Buffer &get_packet() {
        return packet_;
    }

    PacketBuilder &operator<<(uint8_t val) {
        packet_.push_back(val);
        return *this;
    };

    PacketBuilder &operator<<(uint16_t val) {
        packet_.push_back(*(((uint8_t *)&val) + 1));
        packet_.push_back(*((uint8_t *)&val));
        return *this;
    };

    PacketBuilder &operator<<(const std::string &val) {
        std::copy(val.begin(), val.end(), std::back_inserter(packet_));
        packet_.push_back(0);
        return *this;
    };

    PacketBuilder &operator<<(const std::vector<uint8_t> &val) {
        std::copy(val.begin(), val.end(), std::back_inserter(packet_));
        return *this;
    };

    PacketBuilder &operator<<(Mode val) {
        switch (val) {
        case Mode::netascii:
            (*this) << std::string("netascii");
            break;
        case Mode::octet:
            (*this) << std::string("octet");
            break;
        case Mode::mail:
            (*this) << std::string("mail");
            break;
        }
        return *this;
    };
};
} // namespace tftp

#endif