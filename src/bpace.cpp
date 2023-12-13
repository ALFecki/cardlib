#include <bpace.h>

#include <iomanip>

Bpace::Bpace(std::string password, pwd_t pwd_type) : password(password), pwdType(pwd_type) {
    this->logger = Logger::getInstance();
    this->logger->setLogOutput("CONSOLE");
    this->logger->setLogLevel("INFO");
}

int Bpace::bpaceInit() {
    if (pcsc.initPCSC() != 0) {
        logger->log(__FILE__, __LINE__, "Cannot create card context", LogLvl::ERROR);
        return -1;
    }

    if (!this->chooseApplеt(AID_KTA_APPLET, sizeof(AID_KTA_APPLET))) {
        return -1;
    }

    if (!this->chooseMF()) {
        return -1;
    }
    std::vector<octet> initBpace;

    auto encoded = derEncode(0x80, std::vector<octet>(OID_BPACE, OID_BPACE + sizeof(OID_BPACE)));
    std::copy(encoded.begin(), encoded.end(), std::back_inserter(initBpace));

    encoded = derEncode(0x83, std::vector<octet>(1, static_cast<octet>(this->pwdType)));
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
    auto apdu = APDUEncode(APDU(Cla::Default, Instruction::BPACEInit, 0xC1, 0xA4, initBpace));
    auto resp = pcsc.decodeResponse(pcsc.sendCommandToCard(apdu));
    if (resp->sw1 != 0x90 && resp->sw1 != 0x63) {
        logger->log(__FILE__, __LINE__, "Init BPACE failed", LogLvl::ERROR);
        return -1;
    }
    return 0;
}

int Bpace::bPACEStart() {
    auto error = bpaceInit();
    if (error != ERR_OK) {
        return error;
    }

    err_t err = bignParamsStd(&this->params, "1.2.112.0.2.0.34.101.45.3.1");

    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Cannot start BPACE due to std params", LogLvl::ERROR);
        return err;
    }
    prngEchoStart(this->echo, this->params.seed, 8);

    this->blob = blobCreate(9 * this->params.l / 8 + 8 + bakeBPACE_keep(this->params.l));

    this->in = (octet*)this->blob;
    this->out = this->in + 5 * this->params.l / 8;
    this->state = this->out + this->params.l / 2 + 8;

    octet pwd_tmp[16];

    size_t pwdSize = this->password.length();
    std::copy(this->password.begin(), this->password.end(), pwd_tmp);

    err_t code = bakeBPACEStart(this->state, &this->params, &this->settings, pwd_tmp, pwdSize);

    if (code != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Cannot start bpace", LogLvl::ERROR);
        return code;
    }
    return code;
}

bool Bpace::chooseApplеt(const octet aid[], size_t aidSize) {
    std::vector<octet> aidVector(aid, aid + aidSize);
    auto apdu = APDUEncode(APDU(Cla::Default, Instruction::FilesSelect, 0x04, 0x0C, aidVector));
    auto res = pcsc.decodeResponse(pcsc.sendCommandToCard(apdu));
    if (res->sw1 != 0x90) {
        logger->log(__FILE__, __LINE__, "Error in choosing applet", LogLvl::ERROR);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful choosing applet", LogLvl::INFO);
    return true;
}

bool Bpace::chooseMF() {
    auto apdu = APDUEncode(APDU(Cla::Default, Instruction::FilesSelect, 0x00, 0x00));
    auto res = pcsc.decodeResponse(pcsc.sendCommandToCard(apdu));
    if (res->sw1 != 0x90 && res->sw2 != 0x00) {
        logger->log(__FILE__, __LINE__, "Error in choosing MF", LogLvl::ERROR);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful choosing MF", LogLvl::INFO);
    return true;
}

bool Bpace::chooseEF(std::pair<octet, octet> fid) {
    auto card = CardSecure();
    card.initSecure(this->k0);
    auto apdu = card.APDUEncrypt(APDU(Cla::Default, Instruction::ReadData, 0x01, 0x01, {}, 236));
    if (apdu == boost::none) {
        logger->log(__FILE__, __LINE__, "Error in choosing EF: cannot encrypt APDU", LogLvl::ERROR);
        return false;
    }
    auto a = APDUEncode(apdu.get());
    a.push_back(0x00);
    auto res = pcsc.decodeResponse(pcsc.sendCommandToCard(a));
    if (res->sw1 != 0x90 && res->sw2 != 0x00) {
        logger->log(__FILE__, __LINE__, "Error in choosing EF", LogLvl::ERROR);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful choosing EF", LogLvl::INFO);
    return true;
}

std::string Bpace::getName() {
    pcsc.dropContext();
    if (initIdCard()) {
        enterCanToIdCard("334780");
        auto DG = getDG3();
        logger->log(__FILE__, __LINE__, DG, LogLvl::DEBUG);
        return DG;
    }
    // std::cout << chooseEF({0x01, 0x04});
    return "";
}

std::string Bpace::getBirthDate() {
    // pcsc.dropContext();
    // if (initIdCard()) {
    // enterCanToIdCard("334780");
    auto DG = getDG4();
    logger->log(__FILE__, __LINE__, DG, LogLvl::DEBUG);
    return DG;
    // }
    return "";
}

std::string Bpace::getSex() {
    // pcsc.dropContext();
    // if (initIdCard()) {
    // enterCanToIdCard("334780");
    auto DG = getDG5();
    logger->log(__FILE__, __LINE__, DG, LogLvl::DEBUG);
    return DG;
    // }
    return "";
}

std::string Bpace::getIdentityNumber(std::string pin2) {
    pcsc.dropContext();
    if (initIdCard()) {
        enterPin1ToIdCard(this->password);
        // enterPin2ToIdCard(pin2);
        auto DG = getDG1();
        logger->log(__FILE__, __LINE__, DG, LogLvl::DEBUG);
        return DG;
    }
    return "";
}

std::vector<octet> Bpace::createMessage1() {
    std::vector<octet> message1;

    err_t code = bakeBPACEStep2(this->out, this->state);

    if (code != ERR_OK) {
        this->logger->log(
            __FILE__, __LINE__, "Error in BPACE step 2: " + std::to_string(code), LogLvl::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message1;
    }

    std::copy(this->out, this->out + (this->params.l / 8), back_inserter(message1));
    message1 = derEncode(0x7c, derEncode(0x80, message1));
    if (message1.empty()) {
        logger->log(__FILE__, __LINE__, "Internal error", LogLvl::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message1;
    }

    return APDUEncode(APDU(Cla::Chained, Instruction::BPACESteps, 0x00, 0x00, message1));
}

std::vector<octet> Bpace::createMessage3(std::vector<octet> message2) {
    std::vector<octet> message3;
    this->in = new octet[message2.size()];
    std::copy(message2.begin(), message2.end(), this->in);
    prngEchoStart(this->echo, this->params.seed, 8);
    int code = bakeBPACEStep4(this->out, this->in, this->state);

    int err = bakeBPACEStepG(this->k0, this->state);

    if (code != ERR_OK || err != ERR_OK) {
        this->logger->log(__FILE__, __LINE__, "Error in step4 BPACE: " + std::to_string(code), LogLvl::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message3;
    }

    std::copy(this->out, this->out + (this->params.l / 2) + 8, back_inserter(message3));
    message3 = derEncode(0x7c, derEncode(0x82, message3));
    return APDUEncode(APDU(Cla::Default, Instruction::BPACESteps, 0x00, 0x00, message3));
}

bool Bpace::lastAuthStep(std::vector<octet> message3) {
    int err = bakeBPACEStep6(message3.data(), this->state);
    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Error in last step BPACE: " + std::to_string(err), LogLvl::ERROR);
        return false;
    }
    err = bakeBPACEStepG(this->k0, this->state);
    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Error in last step BPACE: " + std::to_string(err), LogLvl::ERROR);
        return false;
    }

    if (this->blob != nullptr) {
        blobClose(this->blob);
        this->blob = nullptr;
    }

    return true;
}

std::shared_ptr<apdu_resp_t> Bpace::sendM1() {
    auto message1 = this->createMessage1();
    if (message1.empty()) {
        return nullptr;
    }
    return pcsc.decodeResponse(pcsc.sendCommandToCard(message1));
}

std::shared_ptr<apdu_resp_t> Bpace::sendM3(std::vector<octet> message2) {
    auto mess = this->createMessage3(message2);
    if (mess.empty()) {
        return nullptr;
    }
    return pcsc.decodeResponse(pcsc.sendCommandToCard(mess));
}

void Bpace::getKey(octet* key0) {
    std::copy(this->k0, this->k0 + 32, key0);
}

std::vector<octet> Bpace::getKey() {
    return std::vector<octet>(this->k0, this->k0 + 32);
}

bool Bpace::authorize() {
    auto status = this->bPACEStart();
    if (status != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Unable to init BPACE", LogLvl::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return false;
    }

    auto resp = this->sendM1();
    if (!resp) {
        logger->log(__FILE__, __LINE__, "Error in BPACE step 1", LogLvl::ERROR);
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 2", LogLvl::DEBUG);
        return false;
    }

    if (resp->sw1 != 0x90) {
        logger->log(__FILE__, __LINE__, "Error in BPACE step 1", LogLvl::ERROR);
        logger->log(__FILE__, __LINE__, "Invalid card", LogLvl::DEBUG);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful BPACE step 1", LogLvl::INFO);

    auto tempDecoded = derDecode(0x7c, resp->rdf, resp->rdf_len);
    auto apduResp = derDecode(0x81, tempDecoded.data(), tempDecoded.size());

    std::vector<octet> message3;
    std::copy(apduResp.begin(), apduResp.end(), std::back_inserter(message3));

    if (message3.empty()) {
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 2", LogLvl::ERROR);
        return false;
    }

    logger->log(__FILE__, __LINE__, "Successful BPACE step 2", LogLvl::INFO);

    resp = this->sendM3(message3);
    if (!resp) {
        logger->log(__FILE__, __LINE__, "Error in BPACE step 3", LogLvl::ERROR);
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 3", LogLvl::DEBUG);
        return false;
    }

    if (resp->sw1 != 0x90) {
        logger->log(__FILE__, __LINE__, "Error in BPACE step 3", LogLvl::ERROR);
        logger->log(__FILE__, __LINE__, "Invalid context", LogLvl::DEBUG);
        return false;
    }

    tempDecoded = derDecode(0x7c, resp->rdf, resp->rdf_len);
    apduResp = derDecode(0x83, tempDecoded.data(), tempDecoded.size());

    logger->log(__FILE__, __LINE__, "Successful BPACE step 3", LogLvl::INFO);

    std::vector<octet> M4(apduResp.size());
    std::copy(apduResp.begin(), apduResp.end(), M4.begin());
    if (M4.empty()) {
        logger->log(__FILE__, __LINE__, "Error in BPACE step 4", LogLvl::ERROR);
        this->logger->log(__FILE__, __LINE__, "Invalid password", LogLvl::DEBUG);
        return false;
    }
    logger->log(__FILE__, __LINE__, "Successful BPACE step 4", LogLvl::INFO);

    if (!lastAuthStep(M4)) {
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Last authetication step", LogLvl::DEBUG);
        return false;
    }
    octet k0[32];
    this->getKey(k0);
    this->secure.initSecure(k0);
    this->logger->log(__FILE__, __LINE__, "Successful authorization", LogLvl::INFO);
    this->logger->log(__FILE__, __LINE__, "Secured connection initialized", LogLvl::DEBUG);
    return true;
}