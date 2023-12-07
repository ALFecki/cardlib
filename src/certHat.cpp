#include <certHat.h>
#include <stdio.h>
#include <iostream>

CertHAT::CertHAT(std::vector<octet> objId, std::vector<octet> access) {
    this->objId = std::vector<octet>(objId);
    this->discretionaryData = std::vector<octet>(access);
}


std::vector<octet> CertHAT::encode() {
    std::vector<octet> res;
    
    std::copy(objId.begin(), objId.end() , std::back_inserter(res));
    std::copy(discretionaryData.begin(), discretionaryData.end() , std::back_inserter(res));
    return res;
}
