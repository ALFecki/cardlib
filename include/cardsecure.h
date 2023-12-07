#include <bee2/defs.h>
#include <bee2/crypto/belt.h>
#include <vector>
#include <logger.h>

class CardSecure {
public:

    void initSecure(octet key0[32]);
private:
    void* state;

    octet key1[32]{}, key2[32]{};

    std::shared_ptr<Logger> logger;

};