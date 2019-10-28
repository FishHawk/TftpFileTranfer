#ifndef TFTP_PARSER_HPP
#define TFTP_PARSER_HPP

#include "Tftp.hpp"

namespace tftp {

TftpRrqPacket *parsing_rrq(Packet &p) {
    if (p.get_word(0) != opcode_rrq)
        return nullptr;

    int len = p.is_legal_string(2);
    if (len < 0)
        return nullptr;

    auto rrq = new TftpRrqPacket(p);
    rrq->offset_filename_ = 2;
    rrq->offset_mode_ = 2 + len + 1;
    return rrq;
};

} // namespace tftp

#endif