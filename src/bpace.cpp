#include <bpace.h>

// auto logger = Logger::getInstance();

Bpace::Bpace() {
    pcsc = PCSC();
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
    apdu_cmd_t* apduCmd = new apdu_cmd_t();
    apduCmd->cla = 0x00;
    apduCmd->ins = cmd;
    apduCmd->p1 = 0x00;
    apduCmd->p2 = 0x00;
    std::move(data.begin(), data.end(), apduCmd->cdf);
    octet apdu[apduCmdEnc(0, apduCmd)];
    apduCmdEnc(apdu, apduCmd);
    return std::vector<octet>(apdu, apdu + sizeof(apdu) / sizeof(octet));
}

std::vector<octet> Bpace::createMessage1() {
    std::vector<octet> message1;

    err_t code = bakeBPACEStep2(this->out, this->state);

    if (code != ERR_OK) {
        this->logger->log(__FILE__, __LINE__, "Error in step2 BPACE: " + std::to_string(code), LogLevel::ERROR);
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
        this->logger->log(__FILE__, __LINE__, "Error in step4 BPACE: " + std::to_string(code), LogLevel::ERROR);
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
