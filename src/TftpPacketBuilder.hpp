#ifndef TFTP_PACKET_BUILDER_HPP
#define TFTP_PACKET_BUILDER_HPP

#include <cstdint>
#include <vector>

namespace tftp {
static const uint16_t opcode_rrq = 1;
static const uint16_t opcode_wrq = 2;
static const uint16_t opcode_data = 3;
static const uint16_t opcode_ack = 4;
static const uint16_t opcode_error = 5;

enum class Mode { netascii,
                  octet,
                  mail };
static const Mode default_mode = Mode::octet;

static const std::map<Mode, const char *> mode_to_string = {
    {Mode::netascii, "netascii"},
    {Mode::octet, "octet"},
    {Mode::mail, "mail"},
};
}  // namespace tftp

namespace tftp {

using Buffer = std::vector<uint8_t>;

class PacketBuilder {
private:
    friend class Parser;

public:
    // TODO: raw_ type need change
    std::vector<uint8_t> raw_;

    const Buffer &get_packet() {
        return raw_;
    }

    void dump() const {
        std::cout << "size: " << raw_.size() << std::endl;

        std::cout << "data: ";
        for (auto &c : raw_) {
            if (isprint(c))
                std::cout << c;
            else
                std::cout << '.';
        }
        std::cout << std::endl;

        std::cout << std::hex;
        std::cout << "hex: ";
        for (auto &c : raw_) {
            std::cout << (int)c << ' ';
        }
        std::cout << std::endl;
        std::cout << std::dec;
    };

    PacketBuilder &operator<<(uint8_t val) {
        raw_.push_back(val);
        return *this;
    };

    PacketBuilder &operator<<(uint16_t val) {
        raw_.push_back(*(((uint8_t *)&val) + 1));
        raw_.push_back(*((uint8_t *)&val));
        return *this;
    };

    PacketBuilder &operator<<(const std::string &val) {
        std::copy(val.begin(), val.end(), std::back_inserter(raw_));
        raw_.push_back(0);
        return *this;
    };

    PacketBuilder &operator<<(const std::vector<uint8_t> &val) {
        std::copy(val.begin(), val.end(), std::back_inserter(raw_));
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
}  // namespace tftp

#endif