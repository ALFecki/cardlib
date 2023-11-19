#ifndef PCSC_H
#define PCSC_H

#include <PCSC/winscard.h>
#include <bee2/defs.h>
#include <cstddef>
#include <vector>
#include <stdio.h>
#include <logger.h>


class PCSC {

public:

    PCSC();
    int initPCSC();
    int checkReaderStatus();
    
    std::vector<octet> sendCommandToCard(std::vector<octet> cmd);


private:

    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders, dwActiveProtocol, dwReaderState;
    SCARDHANDLE hCard;
    SCARD_IO_REQUEST pioSendPci;
    BYTE pbAtr[MAX_ATR_SIZE];

};


#endif