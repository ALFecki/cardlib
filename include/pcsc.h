#ifndef PCSC_H
#define PCSC_H

#include <pcsc-lite/winscard.h>
#include <bee2/defs.h>
#include <bee2/core/apdu.h>
#include <logger.h>
#include <stdio.h>

#include <cstddef>
#include <vector>

class PCSC {
public:
    PCSC();
    int initPCSC();
    int checkReaderStatus();

    std::vector<octet> sendCommandToCard(std::vector<octet> cmd);
    std::shared_ptr<apdu_resp_t> decodeResponse(std::vector<octet> response);

    void waitForCard();
    void dropContext();

// private:
    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders, dwActiveProtocol, dwReaderState;
    SCARDHANDLE hCard;
    SCARD_IO_REQUEST pioSendPci;
    BYTE pbAtr[MAX_ATR_SIZE];

    std::shared_ptr<Logger> logger;

};

#endif