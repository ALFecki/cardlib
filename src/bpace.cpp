#include <bpace.h>

#include <iomanip>

// auto logger = Logger::getInstance();

Bpace::Bpace(std::string password) {
    this->logger = Logger::getInstance();
    this->logger->setLogOutput("CONSOLE");
    this->logger->setLogLevel("INFO");
    auto status = this->bPACEStart(password);
    if (status != ERR_OK) {
        std::cerr << "unable to init bpace: " << status;
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
    }
}

int chooseAppleеt() {
    return 0;
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


std::vector<octet> Bpace::createAPDUCmd(octet cmd, const std::vector<octet>& data) {
    int dataSize = data.size();

    if (dataSize > 255) {
        logger->log(__FILE__, __LINE__, "Cannot execute message1, data is too long", LogLevel::ERROR);
        return std::vector<octet>();
    }
    octet stack[255];
    apdu_cmd_t* apduCmd = (apdu_cmd_t*)stack;
    memSetZero(apduCmd, sizeof(apdu_cmd_t));
    apduCmd->cla = 0x00;
    apduCmd->ins = cmd;
    apduCmd->p1 = 0x00;
    apduCmd->p2 = 0x00;
    apduCmd->cdf_len = data.size();
    apduCmd->rdf_len = 25;
    std::move(data.begin(), data.end(), apduCmd->cdf);
    // memCopy(apduCmd->cdf, data.data(), apduCmd->cdf_len);

    if (!apduCmdIsValid(apduCmd)) {
        logger->log(__FILE__, __LINE__, "APDU command is not valid", LogLevel::ERROR);
        return std::vector<octet>();
    }
    size_t apduSize = apduCmdEnc(0, apduCmd);
    std::vector<octet> apdu(apduSize);
    apduCmdEnc(apdu.data(), apduCmd);

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
    std::vector<octet> decoded = APDU::derDecode(0x81, message2.data(), message2.size());
    this->in = new octet[decoded.size()];
    this->in = decoded.data();
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