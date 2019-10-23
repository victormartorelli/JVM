#ifndef JAVACLASS_H
#define JAVACLASS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct JavaClass JavaClass;

#include "constantPool.h"
#include "attributes.h"
#include "fields.h"
#include "methods.h"

enum AccessFlagsType {
    ACCT_CLASS,
    ACCT_FIELD,
    ACCT_METHOD,
    ACCT_INNERCLASS
};

enum JavaClassStatus {
    CLASS_STA_OK,
    CLASS_STA_UNSPTD_VER,
    CLASS_STA_FILE_CN_BE_OPENED,
    CLASS_STA_INV_SIGN,
    MEM_ALLOC_FAILED,
    INV_CP_COUNT,
    UNXPTD_EOF,
    UNXPTD_EOF_READING_CP,
    UNXPTD_EOF_READING_UTF8,
    UNXPTD_EOF_READING_INTERFACES,
    UNXPTD_EOF_READING_ATTR_INFO,
    INV_UTF8_BYTES,
    INV_CP_INDEX,
    UNKNOWN_CP_TAG,

    INV_ACCESS_FLAGS,
    USE_OF_RSVD_CLASS_ACCESS_FLAGS,
    USE_OF_RSVD_METHOD_ACCESS_FLAGS,
    USE_OF_RSVD_FIELD_ACCESS_FLAGS,

    INV_THIS_CLASS_IDX,
    INV_SUPER_CLASS_IDX,
    INV_INTERFACE_IDX,

    INV_FIELD_DESC_IDX,
    INV_METHOD_DESC_IDX,
    INV_NAME_IDX,
    INV_STRING_IDX,
    INV_CLASS_IDX,
    INV_NAME_AND_TYPE_IDX,
    INV_JAVA_IDENTIFIER,

    ATTR_LEN_MISMATCH,
    ATTR_INV_CONST_VALUE_IDX,
    ATTR_INV_SRC_FILE_IDX,
    ATTR_INV_INNERCLASS_IDXS,
    ATTR_INV_EXC_CLASS_IDX,
    ATTR_INV_CODE_LEN,

    FILE_CONTAINS_UNXPTD_DATA
};

struct JavaClass {
    // General
    FILE* file;
    enum JavaClassStatus status;
    uint8_t classNameMismatch;
    // Java Class Specification
    uint16_t minorVersion, majorVersion;
    uint16_t constantPoolCount;
    cp_info* constantPool;
    uint16_t accessFlags;
    uint16_t thisClass;
    uint16_t superClass;
    uint16_t interfaceCount;
    uint16_t* interfaces;
    uint16_t fieldCount;
    field_info* fields;
    uint16_t methodCount;
    method_info* methods;
    uint16_t attributeCount;
    attribute_info* attributes;
    
    uint16_t staticFieldCount;
    uint16_t instanceFieldCount;

    uint32_t totalBytesRead;
    uint8_t lastTagRead;
    int32_t constantPoolEntriesRead;
    int32_t interfaceEntriesRead;
    int32_t fieldEntriesRead;
    int32_t methodEntriesRead;
    int32_t attributeEntriesRead;
    int32_t validityEntriesChecked;
};

enum AccessFlags {
    ACC_PUBLIC          = 0x1,
    ACC_PRIVATE         = 0x2,
    ACC_PROTECTED       = 0x4,
    ACC_STATIC          = 0x8,
    ACC_FINAL           = 0x10,
    ACC_SUPER           = 0x20,
    ACC_SYNCHRONIZED    = 0x20,
    ACC_BRIDGE          = 0x40,
    ACC_VOLATILE        = 0x40,
    ACC_TRANSIENT       = 0x80,
    ACC_VARARGS         = 0x80,
    ACC_NATIVE          = 0x100,
    ACC_INTERFACE       = 0x200,
    ACC_ABSTRACT        = 0x400,
    ACC_STRICT          = 0x800,
    ACC_SYNTHETIC       = 0x1000,

    ACC_INVALID_CLASS_FLAG_MASK = ~(ACC_PUBLIC | ACC_FINAL | ACC_SUPER | ACC_INTERFACE | ACC_ABSTRACT),

    ACC_INVALID_FIELD_FLAG_MASK = ~(ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL |
                                    ACC_VOLATILE | ACC_TRANSIENT),

    ACC_INVALID_METHOD_FLAG_MASK = ~(ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL |
                                     ACC_SYNCHRONIZED | ACC_NATIVE | ACC_ABSTRACT | ACC_STRICT |
                                     ACC_BRIDGE | ACC_VARARGS | ACC_SYNTHETIC),

    ACC_INVALID_INNERCLASS_FLAG_MASK = ~(ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC |
                                         ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT)
};

int openClassFile(JavaClass*, const char*);
void closeClassFile(JavaClass* jc);
void decodeAccessFlags(uint16_t flags, char* buffer, int32_t buffer_len, enum AccessFlagsType acctype);
void printClassFileInfo(JavaClass* jc);
#endif
