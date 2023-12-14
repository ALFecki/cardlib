#ifndef BPACE_H
#define BPACE_H

#include <apducmd.h>
#include <bee2/core/apdu.h>
#include <bee2/core/blob.h>
#include <bee2/core/der.h>
#include <bee2/core/mem.h>
#include <bee2/core/prng.h>
#include <bee2/crypto/bake.h>
#include <bee2/crypto/bign.h>
#include <certHat.h>
#include <logger.h>
#include <pcsc.h>
#include <utils.h>
#include <cardsecure.h>
#include <iterator>
#include <span>
#include <string>

class Bpace {
public:
    Bpace(std::string password, pwd_t pwd_type);

    int bpaceInit();
    int bPACEStart();
    bool chooseApplеt(const octet aid[], size_t aidSize);
    bool chooseMF();
    bool chooseEF(CardSecure &card);

    // temp
    std::string getName();
    std::string getSex();
    std::string getBirthDate();
    std::string getIdentityNumber(std::string pin2  );

    bool authorize();

    std::vector<octet> createMessage1();
    std::vector<octet> createMessage3(std::vector<octet> message2);
    std::shared_ptr<apdu_resp_t> sendM1();
    std::shared_ptr<apdu_resp_t> sendM3(std::vector<octet> message2);
    bool lastAuthStep(std::vector<octet> message3);
    std::vector<octet> getKey();
    void getKey(octet *key0);

    ~Bpace();

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
    CardSecure secure = CardSecure();
    std::string password;
    pwd_t pwdType;


    std::shared_ptr<Logger> logger;
};

#endif