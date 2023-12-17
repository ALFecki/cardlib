#include <logger.h>
#include <pcsc-lite/winscard.h>
#include <string.h>
#include <vector>

#include <pcsc.h>

#include <cstdint>
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif
#include "cardlib.h"
#ifdef __cplusplus
}
#endif

int32_t transmit(const void* ctx_data, const Data* data, Data* response);
int init_pcsc(PCSC pcsc);
bool initIdCard();
std::string getDG1();
std::string getDG3();
std::string getDG4();
std::string getDG5();
bool enterCanToIdCard(const std::string& can);
bool enterPin2ToIdCard(const std::string& pin);
bool enterPin1ToIdCard(const std::string& pin);

void dropCtx();
