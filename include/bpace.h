#include <bee2/defs.h>
#include <bee2/crypto/bign.h>
#include <bee2/crypto/bake.h>
#include <bee2/core/apdu.h>
#include <bee2/core/mem.h>
#include <bee2/core/prng.h>
#include <bee2/core/blob.h>
#include <iterator>
#include <string>
#include <vector>
#include <logger.h>

class Bpace {


public:
    Bpace(octet pass, octet helloa, octet hellob);
    
    int bPACEStart(std::string password);
    std::vector<unsigned char> getM1();



private:
    bign_params params{};
    octet echo[64]{};
    blob_t blob{};
    octet *in{}, *out{};
    void *state{};


    bake_settings settings = {
            .kca = TRUE,
            .kcb = TRUE,
            .helloa = "",
            .helloa_len = 0,
            .hellob = "",
            .hellob_len = 0,
            .rng = prngEchoStepR,
            .rng_state = echo
    };

    octet password;
    octet helloa;
    octet hellob;
};