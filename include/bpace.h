#ifndef BPACE_H
#define BPACE_H

#include <bee2/core/apdu.h>
#include <bee2/core/blob.h>
#include <bee2/core/der.h>
#include <bee2/core/mem.h>
#include <bee2/core/prng.h>
#include <bee2/crypto/bake.h>
#include <bee2/crypto/bign.h>
#include <bee2/defs.h>

#include <apducmd.h>
#include <logger.h>
#include <pcsc.h>


#include <iterator>
#include <string>
#include <vector>
#include <span>
#include <algorithm>

class Bpace {
public:
    Bpace(std::string password);

    int bPACEStart(std::string password);
    bool authorize();

    std::vector<octet> createMessage1();
    std::vector<octet> createMessage3(std::vector<octet> message2);
    std::vector<octet> sendM1();
    std::vector<octet> sendM3(std::vector<octet> message2);
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