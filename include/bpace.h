#ifndef BPACE_H
#define BPACE_H

#include <bee2/core/apdu.h>
#include <bee2/core/blob.h>
#include <bee2/core/der.h>
#include <bee2/core/mem.h>
#include <bee2/core/prng.h>
#include <bee2/crypto/bake.h>
#include <bee2/crypto/bign.h>

#include <json/json.h>

#include <apducmd.h>
#include <certHat.h>
#include <logger.h>
#include <pcsc.h>
#include <cardsecure.h>
#include <utils.h>

#include <iterator>
#include <string>
#include <span>

class Bpace {
public:
    Bpace(std::string password, Pwd pwd_type);

    int bpaceInit(Pwd pwd_type);
    int bPACEStart(std::string password, Pwd pwd_type);
    bool chooseApplеt(const octet aid[], size_t aidSize);
    bool chooseMF();
    bool chooseEF(std::pair<octet, octet> fid);

    // temp
    std::string getName();


    bool authorize();

    std::vector<octet> createMessage1();
    std::vector<octet> createMessage3(std::vector<octet> message2);
    std::vector<octet> sendM1();
    std::vector<octet> sendM3(std::vector<octet> message2);
    bool lastAuthStep(std::vector<octet> message3);
    std::vector<octet> getKey();
    void getKey(octet *key0);

private:
    bign_params params{};
    octet echo[64]{};
    blob_t blob{};
    octet *in{}, *out{};
    void *state{};
    octet mac[32]{};

    octet k0[32]{};

    bake_settings settings = {.kca = TRUE,
                              .kcb = TRUE,
                              .helloa = "",
                              .helloa_len = 0,
                              .hellob = "",
                              .hellob_len = 0,
                              .rng = prngEchoStepR,
                              .rng_state = echo};

    PCSC pcsc;


    std::shared_ptr<Logger> logger;

};

#endif