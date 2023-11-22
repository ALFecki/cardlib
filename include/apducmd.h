#ifndef APDUCMD_H
#define APDUCMD_H

#include <bee2/core/apdu.h>
#include <bee2/core/der.h>
#include <bee2/core/mem.h>
#include <bee2/defs.h>
#include <enums/apduEnum.h>
#include <logger.h>

#include <vector>

namespace APDU {
// std::shared_ptr<Logger> log = Logger::getInstance();

std::vector<octet> derEncode(u32 tag, const std::vector<octet>& data);
std::vector<octet> derDecode(u32 tag, octet* data, size_t len);
std::vector<octet> createAPDUCmd(
    Cla cla, Instruction cmd, octet p1, octet p2, std::vector<octet> data = {});

}  // namespace APDU

#endif