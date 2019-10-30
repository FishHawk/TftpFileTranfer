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

class Buffer {
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
Buffer &operator+(Buffer &buf, T val) {
    static_assert(sizeof(T) == -1, "unsupported data type");
};

template <>
Buffer &operator+<uint8_t>(Buffer &buf, uint8_t val) {
    buf.data.push_back(val);
    return buf;
};

template <>
Buffer &operator+<uint16_t>(Buffer &buf, uint16_t val) {
    buf.data.push_back(*(((uint8_t *)&val) + 1));
    buf.data.push_back(*((uint8_t *)&val));
    return buf;
};

template <>
Buffer &operator+<std::string>(Buffer &buf, std::string val) {
    std::copy(val.begin(), val.end(), std::back_inserter(buf.data));
    buf.data.push_back(0);
    return buf;
};

template <>
Buffer &operator+<std::vector<uint8_t>>(Buffer &buf, std::vector<uint8_t> val) {
    std::copy(val.begin(), val.end(), std::back_inserter(buf.data));
    return buf;
};

template <>
Buffer &operator+<Mode>(Buffer &buf, Mode val) {
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

class PacketRrq {
    using Options = std::map<std::string, std::string>;

private:
    std::string filename_;
    Mode mode_;
    Options options_;

public:
    PacketRrq(std::string filename, Mode mode = default_mode, Options options = {})
        : filename_(filename), mode_(mode), options_(options) {}

    void serialize(Buffer &buf) {
        buf + opcode_rrq + filename_ + mode_;
        for (auto const &[key, val] : options_) {
            buf + key + val;
        }
    }
};

class PacketWrq {
    using Options = std::map<std::string, std::string>;

private:
    std::string filename_;
    Mode mode_;
    Options options_;

public:
    PacketWrq(std::string filename, Mode mode = default_mode, Options options = {})
        : filename_(filename), mode_(mode), options_(options) {}

    void serialize(Buffer &buf) {
        buf + opcode_wrq + filename_ + mode_;
        for (auto const &[key, val] : options_) {
            buf + key + val;
        }
    }
};

class PacketData {
private:
    uint16_t block_;
    std::vector<uint8_t> data_;

public:
    PacketData(uint16_t block, std::vector<uint8_t> data) : block_(block), data_(std::move(data)) {}

    void serialize(Buffer &buf) {
        buf + opcode_wrq + block_ + data_;
    }
};
// class Packet {
// protected:
//     std::vector<uint8_t> data_;

// public:
//     Packet() {}

//     Packet(std::vector<uint8_t> &data) : data_(std::move(data)) {}

//     void add_byte(uint8_t b) {
//         data_.push_back(b);
//     }

//     uint8_t get_byte(unsigned int offset) {
//         return data_[offset];
//     }

//     void add_word(uint16_t w) {
//         add_byte(*(((uint8_t *)&w) + 1));
//         add_byte(*((uint8_t *)&w));
//     }

//     uint16_t get_word(unsigned int offset) {
//         return (((uint16_t)data_[offset]) << 8) + data_[offset + 1];
//     }

//     void add_string(std::string s) {
//         for (auto &c : s) {
//             add_byte(c);
//         }
//     }

//     int is_legal_string(unsigned int offset) {
//         if (offset >= data_.size())
//             return -1;

//         int i = offset;
//         while (data_[i] != 0 && i < data_.size())
//             i++;
//         if (i == data_.size())
//             return -1;

//         return i - offset;
//     }

//     std::vector<uint8_t> &get_data() {
//         return data_;
//     }
// };

// class TftpAckPacket : public Packet {
// private:
//     uint16_t block_;

// public:
//     TftpAckPacket(uint16_t block) {
//         add_word(opcode_data);

//         block_ = block;
//         add_word(block);
//     }

//     uint16_t get_block() {
//         return get_word(2);
//     }
// };

// /*
// Error Codes
//    Value     Meaning
//    0         Not defined, see error message (if any).
//    1         File not found.
//    2         Access violation.
//    3         Disk full or allocation exceeded.
//    4         Illegal TFTP operation.
//    5         Unknown transfer ID.
//    6         File already exists.
//    7         No such user.
// */
// class TftpErrorPacket : public Packet {
// private:
//     uint16_t error_code_;

// public:
//     TftpErrorPacket(uint16_t error_code, std::string error_msg) {
//         add_word(opcode_data);

//         error_code_ = error_code;
//         add_word(error_code);

//         add_string(error_msg);
//         add_byte(0);
//     }

//     uint16_t get_error_code() {
//         return get_word(2);
//     }

//     std::string get_error_msg() {
//         return std::string((char *)(&data_[4]));
//     }
// };
} // namespace tftp
#endif