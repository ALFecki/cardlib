#include <certHat.h>

CertHAT::CertHAT(const octet objId[], const octet access[]) {
    this->objId = std::vector<octet>(objId, objId + sizeof(objId));
    this->discretionaryData = std::vector<octet>(access, access + sizeof(access));
}


std::vector<octet> CertHAT::encode() {
    std::vector<octet> res;
    std::copy(objId.begin(), objId.end() , std::back_inserter(res));
    std::copy(discretionaryData.begin(), discretionaryData.end() , std::back_inserter(res));
    return res;
}
