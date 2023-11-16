#ifndef PCSC_H
#define PCSC_H

#include <PCSC/winscard.h>
#include <cstddef>
#include <stdio.h>
#include <logger.h>


class PCSC {

public:

    PCSC();
    int init_pcsc();

private:

    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders, dwActiveProtocol;
    SCARDHANDLE hCard;
    SCARD_IO_REQUEST pioSendPci;

};


#endif