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