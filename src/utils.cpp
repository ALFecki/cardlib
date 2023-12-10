#include <utils.h>

#define CHECK(f, rv)             \
    if (SCARD_S_SUCCESS != rv) { \
        printf(f ": %ld\n", rv); \
        return -1;               \
    }

SCARD_IO_REQUEST pioSendPci;
SCARDCONTEXT hCtx;
LPTSTR mszReader;
DWORD dwReader, dwActiveProto;
SCARDHANDLE hCardCtx;
IdCard* ctx_eid = 0;

int32_t transmit(const void* ctx_data, const Data* data, Data* response) {
    SCARDHANDLE hCard = *(SCARDHANDLE*)ctx_data;
    pioSendPci = *SCARD_PCI_T1;

    unsigned long int len = 264;
    unsigned char* buf = (unsigned char*)malloc(len);

    int32_t rv = SCardTransmit(hCard, &pioSendPci, data->data, data->len, NULL, (unsigned char*)buf, &len);

    response->len = len;
    memcpy(response->data, buf, len);
    CHECK("SCardTransmit", rv);
    free(buf);
    return rv;
}

int init_pcsc() {
    LONG rv;

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hCtx);
    CHECK("SCardEstablishContext", rv)

    rv = SCardListReaders(hCtx, NULL, NULL, &dwReader);
    CHECK("SCardListReaders", rv)

    mszReader = static_cast<LPTSTR>(calloc(dwReader, sizeof(char)));
    rv = SCardListReaders(hCtx, NULL, mszReader, &dwReader);
    CHECK("SCardListReaders", rv)

    printf("reader name: %s\n", mszReader);

    rv = SCardConnect(hCtx,
                      mszReader,
                      SCARD_SHARE_SHARED,
                      SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                      &hCardCtx,
                      &dwActiveProto);
    CHECK("SCardConnect", rv)

    switch (dwActiveProto) {
        case SCARD_PROTOCOL_T0:
            pioSendPci = *SCARD_PCI_T0;
            break;

        case SCARD_PROTOCOL_T1:
            pioSendPci = *SCARD_PCI_T1;
            break;
    }
    return 0;
}

bool initIdCard() {
    bool isConnected = false;
    int kta_err = 0;

    if (init_pcsc() == 0) {
        isConnected = true;
    }
    ctx_eid = (struct IdCard*)malloc(2048);
    CertificateCtx* pCertificateCtx;

    id_kta_create_cert_ctx((const CertificateCtx**)&pCertificateCtx);
    kta_err = id_kta_init_raw(&hCardCtx, (const IdCard**)&ctx_eid, pCertificateCtx, transmit, LogLevel::Warn);
    if (!isConnected or kta_err != 0) {
        id_kta_drop_ctx(ctx_eid);
        return false;
    }
    return true;
}

std::string getDG1() {
    Data* DG1 = new Data;
    int32_t errorDG1 = id_kta_get_dg3(ctx_eid, (const Data**)&DG1);
    if (errorDG1) {
    } else {
        std::string info(reinterpret_cast<char*>(DG1->data));
        return info;
    }
}

bool enterCanToIdCard(const std::string& can) {
#ifdef Q_OS_ANDROID

#else
    if (ctx_eid == 0) {
        initIdCard();
    }
#endif
    int32_t can_error =
        id_kta_can_auth(ctx_eid,
                        can.c_str(),
                        (int32_t)EidAccess::DG1 | (int32_t)EidAccess::DG2 | (int32_t)EidAccess::DG3 |
                            (int32_t)EidAccess::DG4 | (int32_t)EidAccess::DG5);

    if (can_error) {
        return false;
    }

    int32_t error = id_kta_select_eid(ctx_eid);
    if (error != 0) {
        return false;
    }
    return true;
}
