#include <pcsc-lite/winscard.h>
#include <string.h>

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
int init_pcsc();
bool initIdCard();
std::string getDG1();
bool enterCanToIdCard(const std::string& can);
