#ifndef TFTP_HPP
#define TFTP_HPP

#include <iostream>
#include <map>

#include "TftpPacketBuilder.hpp"

namespace tftp {
class ReadRequest {
public:
    using Options = std::map<std::string, std::string>;

    static Buffer serialize(const std::string &filename, const Mode mode = default_mode, const Options options = {}) {
        PacketBuilder packet;
        packet << opcode_rrq << filename << mode;
        for (auto const &[key, val] : options) {
            packet << key << val;
        }
        return packet.get_packet();
    }

    const std::string &filename() const { return filename_; }
    const Mode &mode() const { return mode_; }
    const Options &options() const { return options_; }

private:
    friend class Parser;

    std::string filename_;
    Mode mode_;
    Options options_;
};

class WriteRequest {
public:
    using Options = std::map<std::string, std::string>;

    static Buffer serialize(const std::string &filename, const Mode mode = default_mode, const Options options = {}) {
        PacketBuilder builder;
        builder << opcode_wrq << filename << mode;
        for (auto const &[key, val] : options) {
            builder << key << val;
        }
        return builder.get_packet();
    }

    const std::string &filename() const { return filename_; }
    const Mode &mode() const { return mode_; }
    const Options &options() const { return options_; }

private:
    friend class Parser;

    std::string filename_;
    Mode mode_;
    Options options_;
};

class DataMessage {
public:
    static Buffer serialize(const uint16_t block, std::vector<uint8_t> data) {
        PacketBuilder builder;
        builder << opcode_data << block << data;
        return builder.get_packet();
    }

    const uint16_t block() const { return block_; }
    const std::vector<uint8_t> &data() const { return data_; }

private:
    friend class Parser;

    uint16_t block_;
    std::vector<uint8_t> data_;
};

class AckMessage {
public:
    static Buffer serialize(const uint16_t block) {
        PacketBuilder builder;
        builder << opcode_ack << block;
        return builder.get_packet();
    }

    const uint16_t block() const { return block_; }

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
    static Buffer serialize(const uint16_t error_code, const std::string &error_msg = "") {
        PacketBuilder builder;
        builder << opcode_error << error_code << error_msg;
        return builder.get_packet();
    }

    const uint16_t error_code() const { return error_code_; }
    const std::string &error_msg() const { return error_msg_; }

private:
    friend class Parser;

    uint16_t error_code_;
    std::string error_msg_;
};

}  // namespace tftp
#endif