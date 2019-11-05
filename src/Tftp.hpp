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

static const std::map<Mode, const char *> mode_to_string = {
    {Mode::netascii, "netascii"},
    {Mode::octet, "octet"},
    {Mode::mail, "mail"},
};
} // namespace tftp

namespace tftp {

class Packet {
private:
    friend class Parser;

public:
    // TODO: raw_ type need change
    std::vector<uint8_t> raw_;

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

    Packet &operator<<(uint8_t val) {
        raw_.push_back(val);
        return *this;
    };

    Packet &operator<<(uint16_t val) {
        raw_.push_back(*(((uint8_t *)&val) + 1));
        raw_.push_back(*((uint8_t *)&val));
        return *this;
    };

    Packet &operator<<(const std::string &val) {
        std::copy(val.begin(), val.end(), std::back_inserter(raw_));
        raw_.push_back(0);
        return *this;
    };

    Packet &operator<<(const std::vector<uint8_t> &val) {
        std::copy(val.begin(), val.end(), std::back_inserter(raw_));
        return *this;
    };

    Packet &operator<<(Mode val) {
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

class ReadRequest {
public:
    using Options = std::map<std::string, std::string>;

    static Packet serialize(const std::string &filename, const Mode mode = default_mode, const Options options = {}) {
        Packet packet;
        packet << opcode_rrq << filename << mode;
        for (auto const &[key, val] : options) {
            packet << key << val;
        }
        return packet;
    }

    const std::string &filename() { return filename_; }
    const Mode &mode() { return mode_; }
    const Options &options() { return options_; }

private:
    friend class Parser;

    std::string filename_;
    Mode mode_;
    Options options_;
};

class WriteRequest {
public:
    using Options = std::map<std::string, std::string>;

    static Packet serialize(const std::string &filename, const Mode mode = default_mode, const Options options = {}) {
        Packet packet;
        packet << opcode_wrq << filename << mode;
        for (auto const &[key, val] : options) {
            packet << key << val;
        }
        return packet;
    }

    const std::string &filename() { return filename_; }
    const Mode &mode() { return mode_; }
    const Options &options() { return options_; }

private:
    friend class Parser;

    std::string filename_;
    Mode mode_;
    Options options_;
};

class Data {
public:
    static Packet serialize(const uint16_t block, std::vector<uint8_t> data) {
        Packet packet;
        packet << opcode_data << block << data;
        return packet;
    }

    const uint16_t block() { return block_; }
    const std::vector<uint8_t> &data() { return data_; }

private:
    friend class Parser;

    uint16_t block_;
    std::vector<uint8_t> data_;
};

class Ack {
public:
    static Packet serialize(const uint16_t block) {
        Packet packet;
        packet << opcode_ack << block;
        return packet;
    }

    const uint16_t block() { return block_; }

private:
    friend class Parser;

    uint16_t block_;
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
class ErrorResponse {
public:
    static Packet serialize(const uint16_t error_code, const std::string &error_msg = "") {
        Packet packet;
        packet << opcode_error << error_code << error_msg;
        return packet;
    }

    const uint16_t error_code() { return error_code_; }
    const std::string &error_msg() { return error_msg_; }

private:
    friend class Parser;

    uint16_t error_code_;
    std::string error_msg_;
};

} // namespace tftp
#endif