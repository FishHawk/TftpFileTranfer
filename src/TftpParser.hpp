#ifndef TFTP_PARSER_HPP
#define TFTP_PARSER_HPP

#include <algorithm>
#include <cctype>
#include <string>

#include "Tftp.hpp"

namespace tftp {

class Parser {
private:
    std::vector<uint8_t> raw_packet_;
    bool is_valid_ = true;
    unsigned int offset_ = 0;
    uint16_t opcode_ = 0;

    explicit operator bool() const {
        return is_valid_;
    }

    Parser &operator>>(uint16_t &val) {
        if (is_valid_ == false)
            return *this;
        if (raw_packet_.size() - offset_ < 2) {
            is_valid_ = false;
            return *this;
        }
        val = ((uint16_t)raw_packet_[offset_] << 8) + raw_packet_[offset_ + 1];
        offset_ += 2;
        return *this;
    }

    Parser &operator>>(std::string &val) {
        if (is_valid_ == false)
            return *this;

        int pos = offset_;
        while (pos < raw_packet_.size() && raw_packet_[pos] != 0)
            pos++;
        if (pos >= raw_packet_.size()) {
            is_valid_ = false;
            return *this;
        }

        val = std::string((char *)(&raw_packet_[offset_]));
        offset_ = pos + 1;
        return *this;
    }

    Parser &operator>>(Mode &val) {
        if (is_valid_ == false)
            return *this;

        int pos = offset_;
        while (pos < raw_packet_.size() && raw_packet_[pos] != 0)
            pos++;
        if (pos >= raw_packet_.size()) {
            is_valid_ = false;
            return *this;
        }

        std::string str = std::string((char *)(&raw_packet_[offset_]));
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (str == "netascii")
            val = Mode::netascii;
        else if (str == "octet")
            val = Mode::octet;
        else if (str == "mail")
            val = Mode::mail;
        else {
            is_valid_ = false;
            return *this;
        }

        offset_ = pos + 1;
        return *this;
    }

public:
    Parser(std::vector<uint8_t> raw_packet)
        : raw_packet_(std::move(raw_packet)) {
        (*this) >> opcode_;
    }

    bool is_rrq() { return opcode_ == opcode_rrq; }
    bool is_wrq() { return opcode_ == opcode_wrq; }
    bool is_data() { return opcode_ == opcode_data; }
    bool is_ack() { return opcode_ == opcode_ack; }
    bool is_error() { return opcode_ == opcode_error; }

    PacketRrq parser_rrq() {
        if (!is_rrq())
            throw std::invalid_argument("invalid packet format");

        PacketRrq rrq;
        if (!((*this) >> rrq.filename_))
            throw std::invalid_argument("invalid packet format");

        if (!((*this) >> rrq.mode_))
            throw std::invalid_argument("invalid packet format");

        while (offset_ < raw_packet_.size()) {
            std::string key, val;
            if (!((*this) >> key >> val))
                throw std::invalid_argument("invalid packet format");
            rrq.options_[key] = val;
        }

        rrq.data = std::move(raw_packet_);
        return rrq;
    }

    PacketWrq parser_wrq() {
        if (!is_wrq())
            throw std::invalid_argument("invalid packet format");

        PacketWrq wrq;
        if (!((*this) >> wrq.filename_))
            throw std::invalid_argument("invalid packet format");

        if (!((*this) >> wrq.mode_))
            throw std::invalid_argument("invalid packet format");

        while (offset_ < raw_packet_.size()) {
            std::string key, val;
            if (!((*this) >> key >> val))
                throw std::invalid_argument("invalid packet format");
            wrq.options_[key] = val;
        }

        wrq.data = std::move(raw_packet_);
        return wrq;
    }

    PacketData parser_data() {
        if (!is_data())
            throw std::invalid_argument("invalid packet format");

        PacketData data;
        if (!((*this) >> data.block_))
            throw std::invalid_argument("invalid packet format");

        data.data_length_ = raw_packet_.size() - offset_;

        return data;
    }

    PacketAck parser_ack() {
        if (!is_ack())
            throw std::invalid_argument("invalid packet format");

        PacketAck ack;
        if (!((*this) >> ack.block_))
            throw std::invalid_argument("invalid packet format");

        return ack;
    }

    PacketError parser_error() {
        if (!is_error())
            throw std::invalid_argument("invalid packet format");

        PacketError error;
        if (!((*this) >> error.error_code_))
            throw std::invalid_argument("invalid packet format");

        if (!((*this) >> error.error_msg_))
            throw std::invalid_argument("invalid packet format");

        return error;
    }
};

} // namespace tftp

#endif