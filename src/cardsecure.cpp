#include <cardsecure.h>

void CardSecure::initSecure(octet key0[32]) {
    this->logger = Logger::getInstance();

    auto level = std::vector<octet>(12, 0x00);
    auto header = std::vector<octet>(16, 0x00);

    header[15] = 0x01;
    err_t err = beltKRP(this->key1, 0x20, key0, 0x20, level.data(), header.data());

    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Getting key1 error: " + err, LogLevel::ERROR);
        return;
    }

    header[15] = 0x02;
    err = beltKRP(this->key2, 0x20, key0, 0x20, level.data(), header.data());

    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Getting key2 error: " + err, LogLevel::ERROR);
        return;
    }
    logger->log(__FILE__, __LINE__, "Successful keys init" + err, LogLevel::INFO);
}

boost::optional<APDU> CardSecure::APDUEncrypt(APDU command) {
    auto iv = std::vector<octet>(16, 0);
    iv[15] = 0x01;
    octet cla = static_cast<octet>(command.cla) | static_cast<octet>(Cla::Secure);
    std::vector<octet> header{cla, static_cast<octet>(command.instruction), command.p1, command.p2};
    std::vector<octet> y = std::vector<octet>(command.cdf_len, 0);
    beltCFBStart(this->state, this->key2, 32, iv.data());

    if (!command.cdf.empty()) {
        err_t err = beltCFBEncr(y.data(), command.cdf.data(), command.cdf_len, this->key2, 32, iv.data());
        if (err != ERR_OK) {
            logger->log(__FILE__, __LINE__, "CFB encryption error: " + err, LogLevel::ERROR);
            return boost::none;
        }
    }
    y.insert(y.begin(), 0x02);

    auto z = derEncode(0x87, y);

    if (command.le != boost::none) {
        auto der = derEncode(0x97, std::vector<octet>{static_cast<octet>(command.le.get())});
        z.insert(z.end(), der.begin(), der.end());
    }

    std::vector<octet> enc_iy(header);
    enc_iy.insert(enc_iy.end(), z.begin(), z.end());
    beltHMACStart(this->state, this->key1, 32);

    std::vector<octet> mac_payload(iv);
    mac_payload.insert(mac_payload.end(), enc_iy.begin(), enc_iy.end());

    std::vector<octet> t(32, 0);
    err_t err = beltHMAC(t.data(), mac_payload.data(), mac_payload.size(), key1, 32);

    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "HMAC encryption error: " + err, LogLevel::ERROR);
        return boost::none;
    }
    auto enc_t = derEncode(0x8E, t);
    auto cdf = std::vector<octet>(z);
    cdf.insert(cdf.end(), enc_t.begin(), enc_t.end());
    return APDU(static_cast<Cla>(cla), command.instruction, command.p1, command.p2, cdf, 0x00);
}