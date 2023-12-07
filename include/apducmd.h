#ifndef APDUCMD_H
#define APDUCMD_H

#include <bee2/core/apdu.h>
#include <bee2/core/der.h>
#include <bee2/core/mem.h>
#include <bee2/crypto/belt.h>
#include <bee2/defs.h>
#include <enums/apduEnum.h>
#include <logger.h>

#include <boost/optional.hpp>
#include <limits>
#include <vector>

struct APDU {
    Cla cla;
    Instruction instruction;
    octet p1, p2;
    size_t cdf_len = 0;
    std::vector<octet> cdf = {};
    boost::optional<size_t> le;

    APDU(Cla cla, Instruction ins, octet p1, octet p2, std::vector<octet> data = {}, boost::optional<size_t> le = boost::none);
};

std::vector<octet> derEncode(u32 tag, const std::vector<octet>& data);
std::vector<octet> derDecode(u32 tag, octet* data, size_t len);
std::vector<octet> APDUEncode(APDU command);
APDU APDUEncrypt(APDU command);
// std::vector<octet> createAPDUCmd(Cla cla, Instruction cmd, octet p1, octet p2, std::vector<octet> data =
// {});

#endif