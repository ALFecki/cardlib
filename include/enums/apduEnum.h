#ifndef APDUENUM_H
#define APDUENUM_H

#include <bee2/defs.h>

const octet AID_KTA_APPLET[] = {
    0xD1, 0x12, 0x23, 0x52, 0x12, 0x11, 0x11, 0x01, 0x00, 0x01, 0x04, 0x71, 0x01, 0x01, 0x00, 0x00};

const octet OID_EID[] = {0x06, 0x0A, 0x2A, 0x70, 0x00, 0x02, 0x00, 0x22, 0x65, 0x4F, 0x06, 0x01};
const octet OID_ESIGN[] = { 0x06, 0x0A, 0x2A, 0x70, 0x00, 0x02, 0x00, 0x22, 0x65, 0x4F, 0x06, 0x02 };
const octet OID_BPACE[] = { 0x2A, 0x70, 0x00, 0x02, 0x00, 0x22, 0x65, 0x42, 0x15};

const octet EID_ACCESS[] = {0x00, 0x33, 0x6F, 0x7B, 0x10};

enum class Cla { Default = 0x00, Chained = 0x10, Secure = 0x04, SecureChained = 0x14 };

enum class Instruction { FilesSelect = 0xA4, BPACEInit = 0x22, BPACESteps = 0x86 };

#endif