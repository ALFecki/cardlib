#include <apducmd.h>
#include <bee2/crypto/belt.h>
#include <bee2/defs.h>
#include <enums/apduEnum.h>
#include <logger.h>

#include <vector>

class CardSecure {
public:
    void initSecure(octet key0[32]);
    boost::optional<APDU> APDUEncrypt(APDU command);

private:
    void* state;

    octet key1[32]{}, key2[32]{};

    std::shared_ptr<Logger> logger;
};