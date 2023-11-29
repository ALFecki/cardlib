#include <certHat.h>
#include <stdio.h>
#include <iostream>

CertHAT::CertHAT(std::vector<octet> objId, std::vector<octet> access) {
    this->objId = std::vector<octet>(objId);
    this->discretionaryData = std::vector<octet>(access);
}


std::vector<octet> CertHAT::encode() {
    std::vector<octet> res;
    res.push_back(0x7f);
    res.push_back(0x4c);
    res.push_back(0x10);
    
    std::copy(objId.begin(), objId.end() , std::back_inserter(res));
    std::copy(discretionaryData.begin(), discretionaryData.end() , std::back_inserter(res));
    std::cout << "Encoded cert: ";
    for (auto& oc : res) {
        printf("0x%02X ", (unsigned int)(oc));
    }
    std::cout << std::endl;
    return res;
}
