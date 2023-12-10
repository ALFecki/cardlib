#include <apducmd.h>

auto logger = Logger::getInstance();

APDU::APDU(Cla cla, Instruction ins, octet p1, octet p2, std::vector<octet> data, boost::optional<size_t> le)
    : cla(cla), instruction(ins), p1(p1), p2(p2), le(le) {
    if (!data.empty()) {
        this->cdf_len = data.size();
        this->cdf = data;
    }
}

std::vector<octet> derEncode(u32 tag, const std::vector<octet>& data) {
    octet* apduCmd = new octet[2048];
    auto count = derEnc(apduCmd, tag, data.data(), data.size());
    if (count == SIZE_MAX) {
        logger->log(__FILE__, __LINE__, "Error der encode", LogLvl::ERROR);
        throw -1;
    }

    std::vector<octet> res(count);
    std::copy(apduCmd, apduCmd + count, res.begin());
    return res;
}

std::vector<octet> derDecode(u32 tag, octet* data, size_t len) {
    const octet* decoded;
    size_t decodedSize;
    auto count = derDec2(&decoded, &decodedSize, data, len, tag);
    if (count == SIZE_MAX) {
        std::cout << "Error der decoding" << std::endl;
        return std::vector<octet>();
    }
    std::vector<octet> res(decodedSize);
    std::copy(decoded, decoded + decodedSize, res.begin());
    return res;
}

std::vector<octet> APDUEncode(APDU command) {
    int dataSize = command.cdf.size();

    if (dataSize > std::numeric_limits<unsigned short int>::max() - 1) {
        logger->log(__FILE__, __LINE__, "Cannot encode APDU, data is too long", LogLvl::ERROR);
        return std::vector<octet>();
    }

    octet stack[std::numeric_limits<unsigned short int>::max() - 1];
    apdu_cmd_t* apduCmd = (apdu_cmd_t*)stack;
    memSetZero(apduCmd, sizeof(apdu_cmd_t));
    apduCmd->cla = static_cast<octet>(command.cla);
    apduCmd->ins = static_cast<octet>(command.instruction);
    apduCmd->p1 = command.p1;
    apduCmd->p2 = command.p2;
    apduCmd->cdf_len = command.cdf_len;
    std::copy(command.cdf.begin(), command.cdf.end(), apduCmd->cdf);
    if (command.le != boost::none) {
        apduCmd->rdf_len = command.le.get();
    }
    if (!apduCmdIsValid(apduCmd)) {
        logger->log(__FILE__, __LINE__, "APDU command is not valid", LogLvl::ERROR);
        return std::vector<octet>();
    }
    size_t apduSize = apduCmdEnc(0, apduCmd);
    std::vector<octet> apdu(apduSize);
    apduCmdEnc(apdu.data(), apduCmd);
    return apdu;
}