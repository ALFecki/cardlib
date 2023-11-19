#include <include/pcsc.h>

#define CHECK(f, rv)             \
    if (SCARD_S_SUCCESS != rv) { \
        printf(f ": %ld\n", rv); \
        return -1;               \
    }

auto logger = Logger::getInstance();

PCSC::PCSC() {

}


int PCSC::initPCSC() {
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
    logger->log(__FILE__, __LINE__, "Successful pcsc intialization", LogLevel::INFO);
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
    octet* response;
    size_t responseLength;
    result = SCardTransmit(
        hCard,
        &this->pioSendPci,
        cmd.data(),
        cmd.size(),
        NULL,
        response,
        &responseLength
    );
    // проверка

    return std::vector<octet> (response, response + responseLength);
    
}