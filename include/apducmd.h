#ifndef APDUCMD_H
#define APDUCMD_H

#include <bee2/core/der.h>
#include <bee2/defs.h>
#include <logger.h>

#include <vector>

namespace APDU {
// std::shared_ptr<Logger> log = Logger::getInstance();

std::vector<octet> derEncode(u32 tag, const std::vector<octet>& data);
std::vector<octet> derDecode(u32 tag, octet* data, size_t len);

}  // namespace APDU

#endif