#ifndef CERTHAT_H
#define CERTHAT_H

#include <bee2/defs.h>
#include <cstddef>
#include <algorithm>
#include <vector>

class CertHAT {

public:
    CertHAT(std::vector<octet> objId, std::vector<octet> data);
    std::vector<octet> encode();


private:
    std::vector<octet> objId;
    std::vector<octet> discretionaryData;
};


#endif