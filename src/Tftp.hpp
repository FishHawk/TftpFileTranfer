#ifndef TFTP_HPP
#define TFTP_HPP

#include <cstdint>
#include <iostream>
#include <map>
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
} // namespace tftp

namespace tftp {

class Packet {
public:
    std::vector<uint8_t> data;

    void dump() const {
        std::cout << "size: " << data.size() << std::endl;

        std::cout << "data: ";
        for (auto &c : data) {
            if (isprint(c))
                std::cout << c;
            else
                std::cout << '.';
        }
        std::cout << std::endl;

        std::cout << std::hex;
        std::cout << "hex: ";
        for (auto &c : data) {
            std::cout << (int)c << ' ';
        }
        std::cout << std::endl;
        std::cout << std::dec;
    };
};

template <typename T>
Packet &operator+(Packet &buf, T val) {
    static_assert(sizeof(T) == -1, "unsupported data type");
};

template <>
Packet &operator+<uint8_t>(Packet &buf, uint8_t val) {
    buf.data.push_back(val);
    return buf;
};

template <>
Packet &operator+<uint16_t>(Packet &buf, uint16_t val) {
    buf.data.push_back(*(((uint8_t *)&val) + 1));
    buf.data.push_back(*((uint8_t *)&val));
    return buf;
};

template <>
Packet &operator+<std::string>(Packet &buf, std::string val) {
    std::copy(val.begin(), val.end(), std::back_inserter(buf.data));
    buf.data.push_back(0);
    return buf;
};

template <>
Packet &operator+<std::vector<uint8_t>>(Packet &buf, std::vector<uint8_t> val) {
    std::copy(val.begin(), val.end(), std::back_inserter(buf.data));
    return buf;
};

template <>
Packet &operator+<Mode>(Packet &buf, Mode val) {
    switch (val) {
    case Mode::netascii:
        buf + std::string("netascii");
        break;
    case Mode::octet:
        buf + std::string("octet");
        break;
    case Mode::mail:
        buf + std::string("mail");
        break;
    }
    return buf;
};

class PacketRrq : public Packet {
    using Options = std::map<std::string, std::string>;

private:
    std::string filename_;
    Mode mode_;
    Options options_;

public:
    PacketRrq(std::string filename, Mode mode = default_mode, Options options = {})
        : filename_(filename), mode_(mode), options_(options) {
        (*this) + opcode_rrq + filename_ + mode_;
        for (auto const &[key, val] : options_) {
            (*this) + key + val;
        }
    }
};

class PacketWrq : public Packet {
    using Options = std::map<std::string, std::string>;

private:
    std::string filename_;
    Mode mode_;
    Options options_;

public:
    PacketWrq(std::string filename, Mode mode = default_mode, Options options = {})
        : filename_(filename), mode_(mode), options_(options) {
        (*this) + opcode_wrq + filename_ + mode_;
        for (auto const &[key, val] : options_) {
            (*this) + key + val;
        }
    }
};

class PacketData : public Packet {
private:
    uint16_t block_;
    std::vector<uint8_t> data_;

public:
    PacketData(uint16_t block, std::vector<uint8_t> data)
        : block_(block), data_(std::move(data)) {
        (*this) + opcode_data + block_ + data_;
    }
};

class PacketAck : public Packet {
private:
    uint16_t block_;

public:
    PacketAck(uint16_t block)
        : block_(block) {
        (*this) + opcode_data + block_;
    }
};

/*
Error Codes
   Value     Meaning
   0         Not defined, see error message (if any).
   1         File not found.
   2         Access violation.
   3         Disk full or allocation exceeded.
   4         Illegal TFTP operation.
   5         Unknown transfer ID.
   6         File already exists.
   7         No such user.
*/
class PacketError : public Packet {
private:
    uint16_t error_code_;
    std::string error_msg_;

public:
    PacketError(uint16_t error_code, std::string error_msg)
        : error_code_(error_code), error_msg_(error_msg) {
        (*this) + opcode_error + error_code_ + error_msg_;
    }
};

} // namespace tftp
#endif