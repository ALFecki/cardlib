#include <utils.h>

// SCARD_IO_REQUEST pioSendPci;
// SCARDCONTEXT hCtx;
// LPTSTR mszReader;
// DWORD dwReader, dwActiveProto;
// SCARDHANDLE hCardCtx;
PCSC pcscCtx;
IdCard* ctx_eid = 0;
auto loggr = Logger::getInstance();

int32_t transmit(const void* ctx_data, const Data* data, Data* response) {
    SCARDHANDLE hCard = *(SCARDHANDLE*)ctx_data;
    pcscCtx.pioSendPci = *SCARD_PCI_T1;

    unsigned long int len = 264;
    unsigned char* buf = (unsigned char*)malloc(len);

    int32_t rv =
        SCardTransmit(hCard, &pcscCtx.pioSendPci, data->data, data->len, NULL, (unsigned char*)buf, &len);

    response->len = len;
    memcpy(response->data, buf, len);
    free(buf);
    return rv;
}

int init_pcsc(PCSC pcsc) {
    pcscCtx = pcsc;
}

bool initIdCard() {
    int kta_err = 0;
    ctx_eid = (struct IdCard*)malloc(2048);
    CertificateCtx* pCertificateCtx;

    id_kta_create_cert_ctx((const CertificateCtx**)&pCertificateCtx);
    if (pcscCtx.checkReaderStatus()) {
        return false;
    }
    
    kta_err =
        id_kta_init_raw(&pcscCtx.hCard, (const IdCard**)&ctx_eid, pCertificateCtx, transmit, LogLevel::Error);
    if (kta_err != 0) {
        id_kta_drop_ctx(ctx_eid);
        return false;
    }
    return true;
}

std::string getDG1() {
    Data* DG1 = new Data;
    if (pcscCtx.checkReaderStatus()) {
        return "";
    }
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
    if (pcscCtx.checkReaderStatus()) {
        return "";
    }
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
    if (pcscCtx.checkReaderStatus()) {
        return "";
    }
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
    if (pcscCtx.checkReaderStatus()) {
        return "";
    }
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
    // if (ctx_eid == 0) {
    if (!initIdCard()) {
        return false;
    }
    // }
    if (pcscCtx.checkReaderStatus()) {
        return false;
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

    // if (ctx_eid == 0) {
    if (!initIdCard()) {
        return false;
    }
    // }
    isConnected = true;

    if (isConnected) {
        if (pcscCtx.checkReaderStatus()) {
            return false;
        }
        int32_t err_pin_auth =
            id_kta_pin_auth(ctx_eid,
                            pin.c_str(),
                            (int32_t)EidAccess::DG1 | (int32_t)EidAccess::DG2 | (int32_t)EidAccess::DG3 |
                                (int32_t)EidAccess::DG4 | (int32_t)EidAccess::DG5,
                            EsignAccess::BasicMode,
                            2);
        int32_t err_select_eid = id_kta_select_eid(ctx_eid);
        if (err_select_eid != 0) {
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
        // id_kta_drop_ctx(ctx_eid);
        // ctx_eid = 0;
    }
    SCardReleaseContext(pcscCtx.hContext);
}