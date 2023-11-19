#include <bpace.h>

// auto logger = Logger::getInstance();

Bpace::Bpace(std::string password) {
    pcsc = PCSC();

    auto status = this->bPACEStart(password);
}

int Bpace::bPACEStart(std::string pwd) {
    bignParamsStd(&this->params, "1.2.112.0.2.0.34.101.45.3.1");
    prngEchoStart(this->echo, this->params.seed, 8);

    this->blob = blobCreate(9 * this->params.l / 8 + 8 + bakeBPACE_keep(this->params.l));

    this->in = (octet*)this->blob;
    this->out = this->in + 5 * this->params.l / 8;
    this->state = this->out + this->params.l / 2 + 8;

    // start
    octet pwd_tmp[16];
    std::move(pwd.begin(), pwd.end(), pwd_tmp);

    err_t code = bakeBPACEStart(this->state, &this->params, &this->settings, pwd_tmp, pwd.length());

    return code;
}

std::vector<octet> Bpace::createAPDUCmd(octet cmd, std::vector<octet>& data) {
    int dataSize = data.size();

    if (dataSize > 255) {
        logger->log(__FILE__, __LINE__, "Cannot execute message1, data is too long", LogLevel::ERROR);
        return std::vector<octet>();
    }
    octet stack[2048];
    apdu_cmd_t* apduCmd = (apdu_cmd_t*)stack;
    apduCmd->cla = 0x00;
    apduCmd->ins = cmd;
    apduCmd->p1 = 0x00;
    apduCmd->p2 = 0x00;
    apduCmd->cdf_len = data.size();
    apduCmd->rdf_len = 256;

    memCopy(apduCmd->cdf, data.data(), apduCmd->cdf_len);

    size_t apduSize = apduCmdEnc(0, apduCmd);
    std::vector<octet> apdu(apduSize);
    apduCmdEnc(&apdu[0], apduCmd);

    return apdu;
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
    auto apdu = this->createAPDUCmd(0x86, message1);
    std::vector<octet> apduCmd;
    try {
        apduCmd = APDU::derEncode(0x7c, APDU::derEncode(0x80, apdu));
    } catch (int code) {
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message1;
    }
    return apduCmd;
}

std::vector<octet> Bpace::createMessage3(std::vector<octet> message2) {
    std::vector<octet> message3;
    size_t decodedSize;
    this->in = APDU::derDecode(0x81, message2.data(), message2.size()).data();

    prngEchoStart(this->echo, this->params.seed, 8);
    int code = bakeBPACEStep4(this->out, this->in, this->state);

    bakeBPACEStepGA(this->k0, this->k1, this->state);

    if (code != ERR_OK) {
        this->logger->log(
            __FILE__, __LINE__, "Error in step4 BPACE: " + std::to_string(code), LogLevel::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message3;
    }

    std::copy(this->out, this->out + (this->params.l / 2) + 8, back_inserter(message3));
    auto apdu = createAPDUCmd(0x86, message3);
    std::vector<octet> apduCmd;
    try {
        apduCmd = APDU::derEncode(0x7c, APDU::derEncode(0x82, apdu));
    } catch (int code) {
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message3;
    }
    return apduCmd;
}

bool Bpace::lastAuthStep(std::vector<octet> message3) {
    octet* decoded = APDU::derDecode(0x83, message3.data(), message3.size()).data();

    // std::vector<octet> tmp{M4.begin() + 1, M4.end()};
    int err = bakeBPACEStep6(decoded, this->state);
    if (err != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Error in last step BPACE: " + std::to_string(err), LogLevel::ERROR);
        // this->isAuthorized = false;
    } else {
        // this->isAuthorized = true;
    }

    bakeBPACEStepGA(this->k0, this->k1, this->state);

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
    return pcsc.sendCommandToCard(this->createMessage3(message2));
}

void Bpace::getKeys(octet* key0, octet* key1) {
    std::copy(this->k0, this->k0 + 32, key0);
    std::copy(this->k1, this->k1 + 32, key1);
}

bool Bpace::authorize() {
    auto message2 = this->sendM1();
    if (message2.empty()) {
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 2", LogLevel::ERROR);
        return false;
    }

    auto M4 = this->sendM3(message2);
    if (M4.empty()) {
        this->logger->log(__FILE__, __LINE__, "Authorization failed. Message 2", LogLevel::ERROR);
        return false;
    }

    bool isAuthorized = lastAuthStep(M4);
    if (isAuthorized) {
        octet k0[32], k1[32];
        this->getKeys(k0, k1);
        // this->sender->initSecureContext(k0, k1);
    }
    return true;
}