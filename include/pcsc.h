#ifndef PCSC_H
#define PCSC_H

#include <PCSC/winscard.h>


class PCSC {

public:
    int init_pcsc();


private:


    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders;

};


#endif