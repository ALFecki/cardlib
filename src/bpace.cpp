#include <bpace.h>

#include <iomanip>

Bpace::Bpace(std::string password, Pwd pwd_type) {
    this->logger = Logger::getInstance();
    this->logger->setLogOutput("CONSOLE");
    this->logger->setLogLevel("INFO");

    this->chooseApplеt(AID_KTA_APPLET, sizeof(AID_KTA_APPLET));

    this->chooseMF();

    auto status = this->bPACEStart(password, pwd_type);
    if (status != ERR_OK) {
        std::cerr << "unable to init bpace: " << status;
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
    }
}

int Bpace::bpaceInit(Pwd pwd_type) {
    std::vector<octet> initBpace;
    auto encoded = APDU::derEncode(0x80, std::vector<octet>(OID_BPACE, OID_BPACE + sizeof(OID_BPACE)));
    std::copy(encoded.begin(), encoded.end(), std::back_inserter(initBpace));
    encoded = APDU::derEncode(0x83, std::vector<octet>(1, static_cast<octet>(pwd_type)));
    std::copy(encoded.begin(), encoded.end(), std::back_inserter(initBpace));

    auto certHatEsign = CertHAT(std::vector<octet>(OID_ESIGN, OID_ESIGN + sizeof(OID_ESIGN)),
                                std::vector<octet>(ESIGN_ACCESS, ESIGN_ACCESS + sizeof(ESIGN_ACCESS)));

    encoded = certHatEsign.encode();
    std::copy(encoded.begin(), encoded.end(), std::back_inserter(initBpace));

    std::vector<octet> helloa = std::vector<octet>();
    char* certHat = new char[encoded.size()];
    for (size_t i = 0; i < encoded.size(); ++i) {
        certHat[i] = static_cast<const char>(encoded[i]);
    }
    std::copy(certHat, certHat + encoded.size(), std::back_inserter(helloa));

    auto certHatEid = CertHAT(std::vector<octet>(OID_EID, OID_EID + sizeof(OID_EID)),
                              std::vector<octet>(EID_ACCESS, EID_ACCESS + sizeof(EID_ACCESS)));

    encoded = certHatEid.encode();
    std::copy(encoded.begin(), encoded.end(), std::back_inserter(initBpace));

    certHat = new char[encoded.size()];
    for (size_t i = 0; i < encoded.size(); ++i) {
        certHat[i] = static_cast<const char>(encoded[i]);
    }
    std::copy(certHat, certHat + encoded.size(), std::back_inserter(helloa));
    this->settings.helloa = new char[helloa.size()];
    std::copy(helloa.begin(), helloa.end(), (char*)this->settings.helloa);
    this->settings.helloa_len = helloa.size();

    auto apdu = APDU::createAPDUCmd(Cla::Default, Instruction::BPACEInit, 0xC1, 0xA4, initBpace);
    auto resp = pcsc.decodeResponse(pcsc.sendCommandToCard(apdu));
    if (resp->sw1 != 0x90 && resp->sw1 != 0x63) {
        logger->log(__FILE__, __LINE__, "Init BPACE failed", LogLevel::ERROR);
        return -1;
    }
    return 0;
}

int Bpace::bPACEStart(std::string pwd, Pwd pwd_type) {
    auto error = bpaceInit(pwd_type);
    if (error != ERR_OK) {
        return error;
    }

    err_t err = bignParamsStd(&this->params, "1.2.112.0.2.0.34.101.45.3.1");

    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Cannot start BPACE due to std params", LogLevel::ERROR);
        return err;
    }
    prngEchoStart(this->echo, this->params.seed, 8);

    this->blob = blobCreate(9 * this->params.l / 8 + 8 + bakeBPACE_keep(this->params.l));

    this->in = (octet*)this->blob;
    this->out = this->in + 5 * this->params.l / 8;
    this->state = this->out + this->params.l / 2 + 8;

    octet pwd_tmp[16];

    size_t pwdSize = pwd.length();
    std::copy(pwd.begin(), pwd.end(), pwd_tmp);

    err_t code = bakeBPACEStart(this->state, &this->params, &this->settings, pwd_tmp, pwdSize);

    if (code != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Cannot start bpace", LogLevel::ERROR);
        return code;
    }
    return code;
}

bool Bpace::chooseApplеt(const octet aid[], size_t aidSize) {
    std::vector<octet> aidVector(aid, aid + aidSize);
    auto apdu = APDU::createAPDUCmd(Cla::Default, Instruction::FilesSelect, 0x04, 0x0C, aidVector);
    auto res = pcsc.decodeResponse(pcsc.sendCommandToCard(apdu));
    if (res->sw1 != 0x90) {
        logger->log(__FILE__, __LINE__, "Error in choosing applet", LogLevel::ERROR);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful choosing applet", LogLevel::INFO);
    return true;
}

bool Bpace::chooseMF() {
    auto apdu = APDU::createAPDUCmd(Cla::Default, Instruction::FilesSelect, 0x00, 0x00);
    auto res = pcsc.decodeResponse(pcsc.sendCommandToCard(apdu));
    if (res->sw1 != 0x90 && res->sw2 != 0x00) {
        logger->log(__FILE__, __LINE__, "Error in choosing MF", LogLevel::ERROR);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful choosing MF", LogLevel::INFO);
    return true;
}

std::vector<octet> Bpace::createMessage1() {
    std::vector<octet> message1;

    err_t code = bakeBPACEStep2(this->out, this->state);

    if (code != ERR_OK) {
        this->logger->log(
            __FILE__, __LINE__, "Error in step2 BPACE: " + std::to_string(code), LogLevel::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message1;
    }

    std::copy(this->out, this->out + (this->params.l / 8), back_inserter(message1));
    try {
        message1 = APDU::derEncode(0x7c, APDU::derEncode(0x80, message1));
    } catch (int code) {
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message1;
    }
    return APDU::createAPDUCmd(Cla::Chained, Instruction::BPACESteps, 0x00, 0x00, message1);
}

std::vector<octet> Bpace::createMessage3(std::vector<octet> message2) {
    std::vector<octet> message3;
    this->in = new octet[message2.size()];
    std::copy(message2.begin(), message2.end(), this->in);
    prngEchoStart(this->echo, this->params.seed, 8);
    int code = bakeBPACEStep4(this->out, this->in, this->state);

    int err = bakeBPACEStepG(this->k0, this->state);

    if (code != ERR_OK || err != ERR_OK) {
        this->logger->log(
            __FILE__, __LINE__, "Error in step4 BPACE: " + std::to_string(code), LogLevel::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message3;
    }

    std::copy(this->out, this->out + (this->params.l / 2) + 8, back_inserter(message3));
    message3 = APDU::derEncode(0x7c, APDU::derEncode(0x82, message3));
    return APDU::createAPDUCmd(Cla::Default, Instruction::BPACESteps, 0x00, 0x00, message3);
}

bool Bpace::lastAuthStep(std::vector<octet> message3) {
    int err = bakeBPACEStep6(message3.data(), this->state);
    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Error in last step BPACE: " + std::to_string(err), LogLevel::ERROR);
        // this->isAuthorized = false;
    } else {
        // this->isAuthorized = true;
    }

    bakeBPACEStepG(this->k0, this->state);

    if (this->blob != nullptr) {
        blobClose(this->blob);
        this->blob = nullptr;
    }

    return true;
}

std::vector<octet> Bpace::sendM1() {
    return pcsc.sendCommandToCard(this->createMessage1());
}

std::vector<octet> Bpace::sendM3(std::vector<octet> message2) {
    auto mess = this->createMessage3(message2);
    // mess.push_back(0x0c);
    return pcsc.sendCommandToCard(mess);
}

void Bpace::getKey(octet* key0) {
    std::copy(this->k0, this->k0 + 32, key0);
}

std::vector<octet> Bpace::getKey() {
    return std::vector<octet>(this->k0, this->k0 + 32);
}

bool Bpace::authorize() {
    auto m2 = this->sendM1();

    auto resp = pcsc.decodeResponse(m2);

    if (resp->sw1 != 0x90) {
        logger->log(__FILE__, __LINE__, "Error in BPACE step 1", LogLevel::ERROR);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful BPACE step 1", LogLevel::INFO);

    auto tempDecoded = APDU::derDecode(0x7c, resp->rdf, resp->rdf_len);
    auto apduResp = APDU::derDecode(0x81, tempDecoded.data(), tempDecoded.size());

    std::vector<octet> message3;
    std::copy(apduResp.begin(), apduResp.end(), std::back_inserter(message3));

    if (message3.empty()) {
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 2", LogLevel::ERROR);
        return false;
    }

    auto m4 = this->sendM3(message3);
    resp = pcsc.decodeResponse(m4);
    tempDecoded = APDU::derDecode(0x7c, resp->rdf, resp->rdf_len);
    apduResp = APDU::derDecode(0x83, tempDecoded.data(), tempDecoded.size());

    std::vector<octet> M4(apduResp.size());
    std::copy(apduResp.begin(), apduResp.end(), M4.begin());
    if (M4.empty()) {
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 4", LogLevel::ERROR);
        return false;
    }

    bool isAuthorized = lastAuthStep(M4);
    if (isAuthorized) {
        octet k0[32];
        this->getKey(k0);
        // this->sender->initSecureContext(k0, k1);
    }
    this->logger->log(__FILE__, __LINE__, "Successful authorization", LogLevel::INFO);
    return true;
}