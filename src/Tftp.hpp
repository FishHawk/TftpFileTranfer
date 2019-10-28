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
        if (offset >= data_.size()) return -1;

        int i = offset;
        while(data_[i]!=0 && i < data_.size()) i++;
        if (i = data_.size()) return -1;

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
    char *filename_;
    char *mode_;

public:
    TftpReadRequestPacket(std::string filename) {
        add_word(opcode_rrq);

        add_string(filename);
        add_byte(0);

        add_string(default_mode);
        add_byte(0);

        filename_ = (char *)(&data_[2]);
        mode_ = (char *)(&data_[2 + filename.size() + 1]);
    }

    TftpReadRequestPacket(Packet &p) {
        data_ = std::move(p.get_data());
        int len = is_legal_string(2);
        if (len > 0) filename_ = (char *)(&data_[2]);
        else exit(0);
    }

    std::string get_filename() {
        return std::string(filename_);
    }
    std::string get_mode() {
        return std::string(mode_);
    }
};

} // namespace tftp
#endif