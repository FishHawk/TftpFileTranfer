#ifndef TFTP_HPP
#define TFTP_HPP

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

class Packet {
protected:
    std::vector<uint8_t> data;

public:
    Packet() : data(){};

    void add_byte(uint8_t b) {
        data.push_back(b);
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

    void dump() {
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
    }
};

#endif