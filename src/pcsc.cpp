#include <include/pcsc.h>

#define CHECK(f, rv)             \
    if (SCARD_S_SUCCESS != rv) { \
        printf(f ": %ld\n", rv); \
        return -1;               \
    }

auto logger = Logger::getInstance();

PCSC::PCSC() {

}


int PCSC::init_pcsc() {
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
            pioSendPci = *SCARD_PCI_T0;
            break;

        case SCARD_PROTOCOL_T1:
            pioSendPci = *SCARD_PCI_T1;
            break;
    }
    return 0;

}