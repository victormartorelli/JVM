#ifndef VALIDITY_H
#define VALIDITY_H

#include "javaClass.h"

char checkMethodAccessFlags(JavaClass* jc, uint16_t acessFlags);
char checkFieldAccessFlags(JavaClass* jc, uint16_t acessFlags);
char checkClassIdxAndAccessFlags(JavaClass* jc);
char checkClassNameFileNameMatch(JavaClass* jc, const char* classFilePath);
char javaIDIsValid(uint8_t* utf8_bytes, int32_t utf8_len, uint8_t isClassIdentifier);
char nameIdxIsValid(JavaClass* jc, uint16_t name_index, uint8_t isClassIdentifier);
char methodnameIdxIsValid(JavaClass* jc, uint16_t name_index);
char checkCPValidity(JavaClass* jc);

#endif // VALIDITY_H
