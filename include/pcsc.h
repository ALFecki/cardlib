#ifndef PCSC_H
#define PCSC_H

#include <PCSC/winscard.h>
#include <cstddef>
#include <stdio.h>
#include <logger.h>


class PCSC {

public:

    PCSC();
    int initPCSC();
    int checkReaderStatus();
    

private:

    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders, dwActiveProtocol, dwReaderState;
    SCARDHANDLE hCard;
    SCARD_IO_REQUEST pioSendPci;
    BYTE pbAtr[MAX_ATR_SIZE];

};


#endif