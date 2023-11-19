#include <apducmd.h>

// auto logger = Logger::getInstance();

std::vector<octet> APDU::derEncode(u32 tag, const std::vector<octet>& data) {
    octet* apduCmd;
    auto count = derEnc(apduCmd, 0x82, data.data(), data.size());
    if (count == SIZE_MAX) {
        // log->log(__FILE__, __LINE__, "Error der encode", LogLevel::ERROR);
        throw -1;
    }

    return std::vector<octet>(apduCmd, apduCmd + count);
}

std::vector<octet> APDU::derDecode(u32 tag, octet* data, size_t len) {
    const octet* decoded;
    size_t decodedSize;
    auto count = derDec2(&decoded, &decodedSize, data, len, tag);

    return std::vector<octet>(decoded, decoded + count);
}