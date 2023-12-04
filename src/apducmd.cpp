#include <apducmd.h>

auto logger = Logger::getInstance();

std::vector<octet> APDU::derEncode(u32 tag, const std::vector<octet>& data) {
    octet* apduCmd = new octet[2048];
    auto count = derEnc(apduCmd, tag, data.data(), data.size());
    if (count == SIZE_MAX) {
        logger->log(__FILE__, __LINE__, "Error der encode", LogLevel::ERROR);
        throw -1;
    }

    std::vector<octet> res(count);
    std::copy(apduCmd, apduCmd + count, res.begin());
    return res;
}

std::vector<octet> APDU::derDecode(u32 tag, octet* data, size_t len) {
    const octet* decoded;
    size_t decodedSize;
    auto count = derDec2(&decoded, &decodedSize, data, len, tag);
    // std::cout << "Data before decoding: ";
    // for (size_t i = 0; i < len; i++) {
    //     printf("0x%02X ", (unsigned int)(data[i]));
    // }
    // std::cout << std::endl;
    // u32 tag1;
    // auto count = derDec(&tag1, &decoded, &decodedSize, data, len);
    if (count == SIZE_MAX) {
        std::cout << "Error der decoding" << std::endl;
        return std::vector<octet>();
    }
    std::vector<octet> res(decodedSize);
    std::copy(decoded, decoded + decodedSize, res.begin());

    // std::cout << "Data after decoding: ";
    // for (auto oc: res) {
    //     printf("0x%02X ", (unsigned int)(oc));
    // }
    // std::cout << std::endl;
    return res;
}

std::vector<octet> APDU::createAPDUCmd(
    Cla cla, Instruction cmd, octet p1, octet p2, std::vector<octet> data) {
    int dataSize = data.size();

    if (dataSize > 255) {
        logger->log(__FILE__, __LINE__, "Cannot execute message1, data is too long", LogLevel::ERROR);
        return std::vector<octet>();
    }
    octet stack[255];
    apdu_cmd_t* apduCmd = (apdu_cmd_t*)stack;
    memSetZero(apduCmd, sizeof(apdu_cmd_t));
    apduCmd->cla = static_cast<octet>(cla);
    apduCmd->ins = static_cast<octet>(cmd);
    apduCmd->p1 = p1;
    apduCmd->p2 = p2;
    if (!data.empty()) {
        apduCmd->cdf_len = data.size();
        // apduCmd->rdf_len = 255;
        std::move(data.begin(), data.end(), apduCmd->cdf);
    }
    // memCopy(apduCmd->cdf, data.data(), apduCmd->cdf_len);

    if (!apduCmdIsValid(apduCmd)) {
        logger->log(__FILE__, __LINE__, "APDU command is not valid", LogLevel::ERROR);
        return std::vector<octet>();
    }
    size_t apduSize = apduCmdEnc(0, apduCmd);
    std::vector<octet> apdu(apduSize);
    apduCmdEnc(apdu.data(), apduCmd);
    for (auto& oc : apdu) {
        printf("0x%02X ", (unsigned int)(oc));
    }
    std::cout << std::endl;
    return apdu;
}