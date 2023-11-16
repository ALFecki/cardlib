#include <include/pcsc.h>
#include <cstddef>

#define CHECK(f, rv)             \
    if (SCARD_S_SUCCESS != rv) { \
        printf(f ": %ld\n", rv); \
        return -1;               \
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

    printf("reader name: %s\n", mszReaders);

}