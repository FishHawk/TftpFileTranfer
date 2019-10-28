#ifndef TFTP_HPP
#define TFTP_HPP

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

namespace tftp {
static const uint16_t opcode_rrq = 1;
static const uint16_t opcode_wrq = 2;
static const uint16_t opcode_dara = 3;
static const uint16_t opcode_ack = 4;
static const uint16_t opcode_error = 5;

static const std::string default_mode = "octet";
} // namespace tftp

namespace tftp {
class Packet {
protected:
    std::vector<uint8_t> data_;

public:
    Packet() {}

    Packet(std::vector<uint8_t> &data) : data_(std::move(data)) {}

    void add_byte(uint8_t b) {
        data_.push_back(b);
    }

    void add_word(uint16_t w) {
        add_byte(*(((uint8_t *)&w) + 1));
        add_byte(*((uint8_t *)&w));
    }

    void add_string(std::string s) {
        for (auto &c : s) {
            add_byte(c);
        }
    }

    int is_legal_string(unsigned int offset) {
        if (offset >= data_.size())
            return -1;

        int i = offset;
        while (data_[i] != 0 && i < data_.size())
            i++;
        if (i = data_.size())
            return -1;

        return i - offset;
    }

    std::vector<uint8_t> &get_data() {
        return data_;
    }

    void dump() {
        std::cout << "size: " << data_.size() << std::endl;

        std::cout << "data: ";
        for (auto &c : data_) {
            if (isprint(c))
                std::cout << c;
            else
                std::cout << '.';
        }
        std::cout << std::endl;

        std::cout << std::hex;
        std::cout << "hex: ";
        for (auto &c : data_) {
            std::cout << (int)c << ' ';
        }
        std::cout << std::endl;
        std::cout << std::dec;
    }
};

class TftpReadRequestPacket : public Packet {
private:
    unsigned int offset_filename_;
    unsigned int offset_mode_;

public:
    TftpReadRequestPacket(std::string filename) {
        add_word(opcode_rrq);

        offset_filename_ = data_.size();
        add_string(filename);
        add_byte(0);

        offset_mode_ = data_.size();
        add_string(default_mode);
        add_byte(0);
    }

    std::string get_filename() {
        return std::string((char *)(&data_[offset_filename_]));
    }

    std::string get_mode() {
        return std::string((char *)(&data_[offset_mode_]));
    }
};

class TftpWriteRequestPacket : public Packet {
private:
    unsigned int offset_filename_;
    unsigned int offset_mode_;
    unsigned int offset_size_;

public:
    TftpWriteRequestPacket(std::string filename, std::string size) {
        add_word(opcode_wrq);

        offset_filename_ = data_.size();
        add_string(filename);
        add_byte(0);

        offset_mode_ = data_.size();
        add_string(default_mode);
        add_byte(0);

        add_string("tsize");
        add_byte(0);

        offset_size_ = data_.size();
        add_string(size);
        add_byte(0);
    }

    std::string get_filename() {
        return std::string((char *)(&data_[offset_filename_]));
    }

    std::string get_mode() {
        return std::string((char *)(&data_[offset_mode_]));
    }

    std::string get_size() {
        return std::string((char *)(&data_[offset_size_]));
    }
};

} // namespace tftp
#endif