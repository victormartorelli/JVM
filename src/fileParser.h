#ifndef FILEPARSER_H
#define FILEPARSER_H

#include "javaClass.h"

uint8_t readu2(struct JavaClass* jc, uint16_t* out);
uint8_t readu4(struct JavaClass* jc, uint32_t* out);
int32_t readFieldDesc(uint8_t* utf8_bytes, int32_t utf8_len, char checkValidClassID);
int32_t readMethodDesc(uint8_t* utf8_bytes, int32_t utf8_len, char checkValidClassID);
float readFloatFromUint32(uint32_t bytes);
double readDoubleFromUint64(uint64_t bytes);

#endif
