#include "pcsc.h"

#include <iomanip>

PCSC::PCSC() {
    this->logger = Logger::getInstance();
    this->logger->setLogOutput("CONSOLE");
    this->logger->setLogLevel("INFO");
}

int PCSC::initPCSC() {
    logger->log(__FILE__, __LINE__, "PCSC initialization started", LogLvl::INFO);
    LONG result;

    result = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &this->hContext);
    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Cannot initialize card context", LogLvl::WARN);
        return -1;
    }

    result = SCardListReaders(this->hContext, NULL, NULL, &this->dwReaders);
    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Cannot get reader list", LogLvl::WARN);
        return -1;
    }

    this->mszReaders = static_cast<LPTSTR>(calloc(this->dwReaders, sizeof(char)));
    result = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Cannot get reader", LogLvl::WARN);
        return -1;
    }

    logger->log(__FILE__, __LINE__, "Reader name: " + std::string(this->mszReaders), LogLvl::INFO);
    this->waitForCard();

    switch (dwActiveProtocol) {
        case SCARD_PROTOCOL_T0:
            this->pioSendPci = *SCARD_PCI_T0;
            break;

        case SCARD_PROTOCOL_T1:
            this->pioSendPci = *SCARD_PCI_T1;
            break;
    }
    if (this->checkReaderStatus() != SCARD_S_SUCCESS) {
        return -1;
    }

    logger->log(__FILE__, __LINE__, "Successful card context initialization", LogLvl::INFO);
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

    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Cannot check card context", LogLvl::WARN);
        this->dropContext();
        return -1;
    }
    if (!(this->dwReaderState & SCARD_PRESENT)) {
        std::cout << "\033[2A";
        std::cout << "\033[2K";
        std::cout << "\033[2K";
        logger->log(__FILE__, __LINE__, "Cannot find card", LogLvl::WARN);
        this->dropContext();
        return -1;
    }
    logger->log(__FILE__, __LINE__, "Card is in reader", LogLvl::DEBUG);
    return result;
}

std::vector<octet> PCSC::sendCommandToCard(std::vector<octet> cmd) {
    LONG result;
    octet response[255];
    size_t responseLength = sizeof(response);
    result = SCardTransmit(
        this->hCard, &this->pioSendPci, cmd.data(), cmd.size(), NULL, response, &responseLength);
    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Command sending error: " + std::to_string(result), LogLvl::ERROR);
        return std::vector<octet>();
    }
    return std::vector<octet>(response, response + responseLength);
}

std::shared_ptr<apdu_resp_t> PCSC::decodeResponse(std::vector<octet> response) {
    apdu_resp_t* resp = new apdu_resp_t[2048];
    apduRespDec(resp, response.data(), response.size());
    std::shared_ptr<apdu_resp_t> apduResp(resp);
    return apduResp;
}

void PCSC::waitForCard() {
    SCARD_READERSTATE rgReaderState[1];
    rgReaderState[0].szReader = this->mszReaders;
    rgReaderState[0].dwCurrentState = SCARD_STATE_UNAWARE;

    LONG rv = SCardGetStatusChange(this->hContext, INFINITE, rgReaderState, 1);

    if (rgReaderState[0].dwEventState & SCARD_STATE_EMPTY) {
        rgReaderState[0].dwCurrentState = rgReaderState[0].dwEventState;
        logger->log(__FILE__, __LINE__, "Waiting for card...", LogLvl::WARN);
        rv = SCardGetStatusChange(this->hContext, INFINITE, rgReaderState, 1);
        if (rv == SCARD_S_SUCCESS) {
            logger->log(__FILE__, __LINE__, "Card is presented now", LogLvl::DEBUG);
        }
    }
    LONG result = SCardConnect(this->hContext,
                               mszReaders,
                               SCARD_SHARE_SHARED,
                               SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                               &hCard,
                               &dwActiveProtocol);
    if (result != SCARD_S_SUCCESS) {
        logger->log(__FILE__, __LINE__, "Cannot get card context", LogLvl::WARN);
        return;
    }
}

void PCSC::dropContext() {
    SCardReleaseContext(this->hContext);
}