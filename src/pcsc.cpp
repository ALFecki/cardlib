#include "pcsc.h"

#include <iomanip>

#define CHECK(f, rv)             \
    if (SCARD_S_SUCCESS != rv) { \
        printf(f ": %ld\n", rv); \
        return -1;               \
    }

// auto logger = Logger::getInstance();

PCSC::PCSC() {
    this->logger = Logger::getInstance();
    this->logger->setLogOutput("CONSOLE");
    this->logger->setLogLevel("INFO");
    this->initPCSC();
}

int PCSC::initPCSC() {
    logger->log(__FILE__, __LINE__, "PCSC initialization started", LogLevel::INFO);
    LONG result;

    result = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &this->hContext);
    CHECK("SCardEstablishContext", result)

    result = SCardListReaders(this->hContext, NULL, NULL, &this->dwReaders);
    CHECK("SCardListReaders", result)

    this->mszReaders = static_cast<LPTSTR>(calloc(this->dwReaders, sizeof(char)));
    result = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
    CHECK("SCardListReaders", result)
    logger->log(__FILE__, __LINE__, "Reader name: " + std::string(this->mszReaders), LogLevel::INFO);

    result = SCardConnect(this->hContext,
                          mszReaders,
                          SCARD_SHARE_SHARED,
                          SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                          &hCard,
                          &dwActiveProtocol);
    CHECK("SCardConnect", result)

    switch (dwActiveProtocol) {
        case SCARD_PROTOCOL_T0:
            this->pioSendPci = *SCARD_PCI_T0;
            break;

        case SCARD_PROTOCOL_T1:
            this->pioSendPci = *SCARD_PCI_T1;
            break;
    }
    logger->log(__FILE__, __LINE__, "Successful pcsc initialization", LogLevel::INFO);
    return 0;
}

int PCSC::checkReaderStatus() {
    DWORD dwAtrLen = sizeof(this->pbAtr);
    LONG result = SCardStatus(this->hCard,
                              this->mszReaders,
                              &this->dwReaders,
                              &this->dwReaderState,
                              &this->dwActiveProtocol,
                              this->pbAtr,
                              &dwAtrLen);
    logger->log(__FILE__, __LINE__, "Successful pcsc intialization", LogLevel::INFO);
    CHECK("SCardStatus", result);
    return result;
}

std::vector<octet> PCSC::sendCommandToCard(std::vector<octet> cmd) {
    LONG result;
    octet response[255];
    size_t responseLength = sizeof(response);
    result = SCardTransmit(
        this->hCard, &this->pioSendPci, cmd.data(), cmd.size(), NULL, response, &responseLength);
    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Command sending error: " + std::to_string(result), LogLevel::ERROR);
        return std::vector<octet>();
    }
    return std::vector<octet>(response, response + responseLength);
}

std::shared_ptr<apdu_resp_t> PCSC::decodeResponse(std::vector<octet> response) {
    auto decodedSize = apduRespDec(0, response.data(), response.size());
    std::shared_ptr<apdu_resp_t> apduResp = std::make_shared<apdu_resp_t>(decodedSize);
    apduRespDec(apduResp.get(), response.data(), response.size());
    std::cout << "RDF of decoded response: ";
    for (size_t i = 0; i < apduResp->rdf_len; i++) {
        printf("0x%02X ", (unsigned int)(apduResp->rdf[i]));
    }
    std::cout << std::endl;
    return std::shared_ptr<apdu_resp_t>(apduResp);
}