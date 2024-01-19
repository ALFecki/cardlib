#include <cardsecure.h>

void CardSecure::initSecure(octet key0[32]) {
    this->logger = Logger::getInstance();
    this->counter = 0;
    auto level = std::vector<octet>(12, 0x00);
    auto header = std::vector<octet>(16, 0xFF);
    
    
    octet* stack = new octet[btokSM_keep()];
    this->state = (void*)stack;

    btokSMStart(this->state, key0);
    
    
    // octet* stack = new octet[beltKRP_keep()];
    // this->state = (void*)stack;
    // beltKRPStart(this->state, key0, 32, level.data());
    // beltKRPStepG(this->key1, 32, header.data(), this->state);
    // // header[15] = 0x01;
    // // err_t err = beltKRP(this->key1, 0x20, key0, 0x20, level.data(), header.data());

    // // if (err != ERR_OK) {
    // //     logger->log(__FILE__, __LINE__, "Getting key1 error: " + err, LogLevel::ERROR);
    // //     return;
    // // }

    // header[15] = 0x00;
    // header[14] = 0x00;
    // header[13] = 0x00;
    // header[12] = 0x00;
    // stack = new octet[beltKRP_keep()];
    // this->state = (void*)stack;
    // beltKRPStart(this->state, key0, 32, level.data());
    // beltKRPStepG(this->key2, 32, header.data(), this->state);
    // err_t err = beltKRP(this->key2, 0x20, key0, 0x20, level.data(), header.data());

    // if (err != ERR_OK) {
    //     logger->log(__FILE__, __LINE__, "Getting key2 error: " + err, LogLevel::ERROR);
    //     return;
    // }
    logger->log(__FILE__, __LINE__, "Successful keys init", LogLevel::INFO);
}


boost::optional<APDU> CardSecure::APDUEncrypt(APDU command) {


    btokSMCtrInc(this->state);
    auto cmd = new octet[1024];
    size_t cmdLen;

    btokSMCmdWrap(cmd, &cmdLen, APDUEncode(command).data(), this->state);
    // ++this->counter;
    // auto counterArr = static_cast<octet*>(static_cast<void*>(&this->counter));
    // std::vector<octet> iv(counterArr, counterArr + 16);
    // octet cla = static_cast<octet>(command.cla) | static_cast<octet>(Cla::Secure);
    // std::vector<octet> header{cla, static_cast<octet>(command.instruction), command.p1, command.p2};
    // std::vector<octet> y = std::vector<octet>(command.cdf);

    // if (!command.cdf.empty()) {
    //     octet* stack = new octet[beltCFB_keep()];
    //     this->state = (void*)stack;
    //     beltCFBStart(this->state, this->key1, 32, iv.data());
    //     beltCFBStepE(y.data(), y.size(), this->state);
    //     // err_t err = beltCFBEncr(y.data(), command.cdf.data(), command.cdf_len, this->key2, 32, iv.data());
    //     // if (err != ERR_OK) {
    //     //     logger->log(__FILE__, __LINE__, "CFB encryption error: " + err, LogLevel::ERROR);
    //     //     return boost::none;
    //     // }
    // }
    // auto z = std::vector<octet>();
    
    // if (!y.empty()) {
    //     y.insert(y.begin(), 0x02);
    //     z = derEncode(0x87, y);
    // }


    // if (command.le != boost::none) {
    //     auto der = derEncode(0x97, std::vector<octet>{static_cast<octet>(command.le.get())});
    //     z.insert(z.end(), der.begin(), der.end());
    // }

    // std::vector<octet> enc_iy(header);
    // enc_iy.insert(enc_iy.end(), z.begin(), z.end());
    // octet* stack = new octet[beltHMAC_keep()];
    // this->state = (void*)stack;
    // beltHMACStart(this->state, this->key1, 32);
    

    // std::vector<octet> mac_payload(iv);
    // mac_payload.insert(mac_payload.end(), enc_iy.begin(), enc_iy.end());

    // std::vector<octet> t(8, 0);
    // beltHMACStepA(mac_payload.data(), mac_payload.size(), this->state);
    // beltHMACStepG2(t.data(), 8, this->state);
    // // err_t err = beltHMAC(t.data(), mac_payload.data(), mac_payload.size(), key1, 32);

    // if (!beltHMACStepV2(t.data(), 8, this->state)) {
    //     logger->log(__FILE__, __LINE__, "HMAC encryption error", LogLevel::ERROR);
    //     return boost::none;
    // }
    // auto enc_t = derEncode(0x8E, t);
    // auto cdf = std::vector<octet>(z);
    // cdf.insert(cdf.end(), enc_t.begin(), enc_t.end());
    // return APDU(static_cast<Cla>(cla), command.instruction, command.p1, command.p2, cdf);
}