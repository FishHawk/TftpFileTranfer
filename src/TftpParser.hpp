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
        PacketRrq rrq;
        // rrq = new PacketRrq();
        if (!((*this) >> rrq.filename_))
            throw std::invalid_argument("invalid packet format");

        if (!((*this) >> rrq.mode_))
            throw std::invalid_argument("invalid packet format");

        while (offset_ < raw_packet_.size()) {
            std::string key, val;
            if ((*this) >> key >> val)
                throw std::invalid_argument("invalid packet format");
            rrq.options_[key] = val;
        }

        rrq.data = std::move(raw_packet_);
        return rrq;
    }
}; // namespace tftp

} // namespace tftp

#endif