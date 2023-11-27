#ifndef BPACE_H
#define BPACE_H

#include <bee2/core/apdu.h>
#include <bee2/core/blob.h>
#include <bee2/core/der.h>
#include <bee2/core/mem.h>
#include <bee2/core/prng.h>
#include <bee2/crypto/bake.h>
#include <bee2/crypto/bign.h>

#include <apducmd.h>
#include <certHat.h>
#include <logger.h>
#include <pcsc.h>


#include <iterator>
#include <string>
#include <span>

class Bpace {
public:
    Bpace(std::string password);

    int bpaceInit();
    int bPACEStart(std::string password);
    std::vector<octet> chooseApplеt(const octet aid[], size_t aidSize);
    bool chooseMF();

    bool authorize();

    std::vector<octet> createMessage1();
    std::vector<octet> createMessage3(std::vector<octet> message2);
    apdu_resp_t sendM1();
    apdu_resp_t sendM3(std::vector<octet> message2);
    bool lastAuthStep(std::vector<octet> message3);
    void getKeys(octet *key0, octet *key1);

private:
    bign_params params{};
    octet echo[64]{};
    blob_t blob{};
    octet *in{}, *out{};
    void *state{};

    octet k0[32]{}, k1[32]{};

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