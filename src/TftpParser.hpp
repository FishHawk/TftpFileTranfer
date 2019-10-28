#ifndef TFTP_PARSER_HPP
#define TFTP_PARSER_HPP

#include "Tftp.hpp"

namespace tftp {

TftpRrqPacket *parsing_rrq(Packet &p) {
    if (p.get_word(0) != opcode_rrq)
        return nullptr;

    auto rrq = new TftpRrqPacket(p);
    int offset, length;

    offset = 2;
    length = rrq->is_legal_string(offset);
    if (length < 0) {
        delete rrq;
        return nullptr;
    }
    rrq->offset_filename_ = offset;

    offset += length + 1;
    length = rrq->is_legal_string(offset);
    if (length < 0) {
        delete rrq;
        return nullptr;
    }
    rrq->offset_mode_ = offset;

    return rrq;
};

TftpWrqPacket *parsing_wrq(Packet &p) {
    if (p.get_word(0) != opcode_wrq)
    return nullptr;

    auto wrq = new TftpWrqPacket(p);
    int offset, length;

    offset = 2;
    length = wrq->is_legal_string(offset);
    if (length < 0) {
        delete wrq;
        return nullptr;
    }
    wrq->offset_filename_ = offset;

    offset += length + 1;
    length = wrq->is_legal_string(offset);
    if (length < 0) {
        delete wrq;
        return nullptr;
    }
    wrq->offset_mode_ = offset;

    offset += length + 1;
    length = wrq->is_legal_string(offset);
    if (length < 0) {
        delete wrq;
        return nullptr;
    }

    offset += length + 1;
    length = wrq->is_legal_string(offset);
    if (length < 0) {
        delete wrq;
        return nullptr;
    }
    wrq->offset_size_ = offset;
    return wrq;
};
} // namespace tftp

#endif