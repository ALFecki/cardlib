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
auto loggr = Logger::getInstance();

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
    int32_t errorDG1 = id_kta_get_dg1(ctx_eid, (const Data**)&DG1);
    if (errorDG1) {
        loggr->log(__FILE__, __LINE__, "Cannot get data groups", LogLvl::ERROR);
    } else {
        loggr->log(__FILE__, __LINE__, "Successful choosing EF", LogLvl::INFO);
        loggr->log(__FILE__, __LINE__, "Data groups received from card ", LogLvl::INFO);
        auto d = std::vector<uint8_t>(DG1->data, DG1->data + DG1->len);
        std::string info(reinterpret_cast<char*>(d.data()));
        loggr->log(__FILE__, __LINE__, "Successful decryption data groups", LogLvl::INFO);
        return info;
    }
    delete DG1;
}

std::string getDG3() {
    Data* DG3 = new Data;
    int32_t errorDG1 = id_kta_get_dg3(ctx_eid, (const Data**)&DG3);
    if (errorDG1) {
        loggr->log(__FILE__, __LINE__, "Cannot get data groups", LogLvl::ERROR);
    } else {
        loggr->log(__FILE__, __LINE__, "Successful choosing EF", LogLvl::INFO);
        loggr->log(__FILE__, __LINE__, "Data groups received from card ", LogLvl::INFO);
        std::string info(reinterpret_cast<char*>(DG3->data));
        loggr->log(__FILE__, __LINE__, "Successful decryption data groups", LogLvl::INFO);
        return info;
    }
    delete DG3;
}

std::string getDG4() {
    Data* DG4 = new Data;
    int32_t errorDG1 = id_kta_get_dg4(ctx_eid, (const Data**)&DG4);
    if (errorDG1) {
        loggr->log(__FILE__, __LINE__, "Cannot get data groups", LogLvl::ERROR);
        return "";
    } else {
        loggr->log(__FILE__, __LINE__, "Successful choosing EF", LogLvl::INFO);
        loggr->log(__FILE__, __LINE__, "Data groups received from card ", LogLvl::INFO);
        auto d = std::vector<uint8_t>(DG4->data, DG4->data + DG4->len);
        std::string info(reinterpret_cast<char*>(d.data()));
        loggr->log(__FILE__, __LINE__, "Successful decryption data groups", LogLvl::INFO);
        return info;
    }
    delete DG4;
}

std::string getDG5() {
    Data* DG5 = new Data;
    int32_t errorDG1 = id_kta_get_dg5(ctx_eid, (const Data**)&DG5);
    if (errorDG1) {
        loggr->log(__FILE__, __LINE__, "Cannot get data groups", LogLvl::ERROR);
    } else {
        loggr->log(__FILE__, __LINE__, "Successful choosing EF", LogLvl::INFO);
        loggr->log(__FILE__, __LINE__, "Data groups received from card ", LogLvl::INFO);
        std::string info(reinterpret_cast<char*>(DG5->data));
        loggr->log(__FILE__, __LINE__, "Successful decryption data groups", LogLvl::INFO);
        return info;
    }
    delete DG5;
}

bool enterCanToIdCard(const std::string& can) {
    if (ctx_eid == 0) {
        initIdCard();
    }
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

bool enterPin1ToIdCard(const std::string& pin) {
    bool isConnected = false;

    if (ctx_eid == 0) {
        initIdCard();
        isConnected = true;
    } else {
        isConnected = true;
    }

    if (isConnected) {
        int32_t err_pin_auth =
            id_kta_pin_auth(ctx_eid,
                            pin.c_str(),
                            (int32_t)EidAccess::DG1 | (int32_t)EidAccess::DG2 | (int32_t)EidAccess::DG3 |
                                (int32_t)EidAccess::DG4 | (int32_t)EidAccess::DG5,
                            EsignAccess::BasicMode,
                            2);

        // qDebug() << err_pin_auth;
        // switch (err_pin_auth) {
        //     case 0x0:
        //         break;
        //     case 0x6983:
        //         qDebug() << "Error: Пароль частично заблокирован";
        //         close_idcard();
        //         return false;
        //     case 0x6985:
        //         qDebug() << "Error: Пароль приостановлен";
        //         close_idcard();
        //         return false;
        //     case 0x63C2:
        //         qDebug() << "Error: Ошибка ПИН кода 1. Осталось 2 попытки";
        //         close_idcard();
        //         return false;
        //     case 0x63C1:
        //         qDebug() << "Error: Ошибка ПИН кода 1. Осталось 1 попытка";
        //         close_idcard();
        //         return false;
        //     default:
        //         qDebug() << "Error: Ошибка пинкода";
        //         close_idcard();
        //         return false;
        // }
        int32_t err_select_esign = id_kta_select_eid(ctx_eid);
        if (err_select_esign != 0) {
            return false;
        }
        return true;
    } else {
        id_kta_drop_ctx(ctx_eid);
        return false;
    }
}

bool enterPin2ToIdCard(const std::string& pin) {
    bool isConnected = false;

    if (ctx_eid == 0) {
        isConnected = false;
    } else {
        isConnected = true;
    }

    if (isConnected) {
        int32_t err_pin2_verify = id_kta_pin2_verify(ctx_eid, pin.c_str());
        std::cout << err_pin2_verify;
        if (err_pin2_verify != 0) {
            id_kta_drop_ctx(ctx_eid);
            return false;
        }
        return true;

    } else {
        id_kta_drop_ctx(ctx_eid);
        return false;
    }
}

void dropCtx() {
    if (ctx_eid != 0) {
        id_kta_drop_ctx(ctx_eid);
    }
}