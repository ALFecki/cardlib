#include <bee2/defs.h>
#include <bee2/crypto/belt.h>
#include <vector>
#include <logger.h>
#include <apducmd.h>
#include <enums/apduEnum.h>

class CardSecure {
public:

    void initSecure(octet key0[32]);
    boost::optional<APDU> APDUEncrypt(APDU command);
private:
    void* state;

    octet key1[32]{}, key2[32]{};

    std::shared_ptr<Logger> logger;

};