#include <bpace.h>

auto logger = Logger::getInstance();

Bpace::Bpace(octet pass, octet helloa, octet hellob) {

}


int Bpace::bPACEStart(std::string pwd) {
    bignParamsStd(&this->params, "1.2.112.0.2.0.34.101.45.3.1");
    prngEchoStart(this->echo, this->params.seed, 8);

    this->blob = blobCreate(9 * this->params.l / 8 + 8 + bakeBPACE_keep(this->params.l));

    this->in = (octet *) this->blob;
    this->out = this->in + 5 * this->params.l / 8;
    this->state = this->out + this->params.l / 2 + 8;

    // start
    octet pwd_tmp[16];
    std::move(pwd.begin(), pwd.end(), pwd_tmp);

    err_t code = bakeBPACEStart(this->state, &this->params, &this->settings, pwd_tmp, pwd.length());

    return code;
}

std::vector<unsigned char> Bpace::getM1() {
    std::vector<unsigned char> message1;

    err_t code = bakeBPACEStep2(this->out, this->state);

    if (code != ERR_OK) {
        logger->log(__FILE__, __LINE__, "Error in step2 BPACE: " + std::to_string(code), LogLevel::ERROR);
        if (this->blob != nullptr) {
            blobClose(this->blob);
            this->blob = nullptr;
        }
        return message1;
    }

    std::copy(this->out, this->out + (this->params.l / 8), back_inserter(message1));


}

std::vector<unsigned char> createAPDUCmd(unsigned char cmd, std::vector<unsigned char> &data) {
    int dataSize = data.size();

    if (dataSize > 255) {
        logger->log(__FILE__, __LINE__, "Cannot execute message1, data is too long", LogLevel::ERROR);
        return std::vector<unsigned char>();
    } else {
        apdu_cmd_t* apduCmd = new apdu_cmd_t();
        apduCmd->cla = 0x00; apduCmd->ins = cmd; apduCmd->p1 = 0x00; apduCmd->p2 = 0x00;
        
        // apduCmd->cdf = reinterpret_cast<octet*>(data.data());
        
    }
}


