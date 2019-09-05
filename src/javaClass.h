#ifndef JAVACLASS_H
#define JAVACLASS_H
#include <stdio.h>
#include <stdint.h>
#include "constantPool.h"

enum JavaClassStatus {
    CLASS_STATUS_OK,
    CLASS_STATUS_UNSUPPORTED_VERSION,
    CLASS_STATUS_FILE_COULDNT_BE_OPENED,
    CLASS_STATUS_INVALID_SIGNATURE,
    MEMORY_ALLOCATION_FAILED,
    INVALID_CONSTANT_POOL_COUNT,
    UNEXPECTED_EOF,
    UNEXPECTED_EOF_READING_CONSTANT_POOL,
    UNEXPECTED_EOF_READING_UTF8,
    UNEXPECTED_EOF_READING_INTERFACES,
    UNEXPECTED_EOF_READING_ATTRIBUTE_INFO,
    INVALID_UTF8_BYTES,
    INVALID_CONSTANT_POOL_INDEX,
    UNKNOWN_CONSTANT_POOL_TAG,

    INVALID_ACCESS_FLAGS,
    USE_OF_RESERVED_CLASS_ACCESS_FLAGS,
    USE_OF_RESERVED_METHOD_ACCESS_FLAGS,
    USE_OF_RESERVED_FIELD_ACCESS_FLAGS,

    INVALID_THIS_CLASS_INDEX,
    INVALID_SUPER_CLASS_INDEX,
    INVALID_INTERFACE_INDEX,

    INVALID_FIELD_DESCRIPTOR_INDEX,
    INVALID_METHOD_DESCRIPTOR_INDEX,
    INVALID_NAME_INDEX,
    INVALID_STRING_INDEX,
    INVALID_CLASS_INDEX,
    INVALID_NAME_AND_TYPE_INDEX,
    INVALID_JAVA_IDENTIFIER,

    ATTRIBUTE_LENGTH_MISMATCH,
    ATTRIBUTE_INVALID_CONSTANTVALUE_INDEX,
    ATTRIBUTE_INVALID_SOURCEFILE_INDEX,
    ATTRIBUTE_INVALID_INNERCLASS_INDEXES,
    ATTRIBUTE_INVALID_EXCEPTIONS_CLASS_INDEX,
    ATTRIBUTE_INVALID_CODE_LENGTH,

    FILE_CONTAINS_UNEXPECTED_DATA
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
    // Class Data Info
    uint16_t staticFieldCount;
    uint16_t instanceFieldCount;
    // Debug info
    uint32_t totalBytesRead;
    uint8_t lastTagRead;
    int32_t constantPoolEntriesRead;
    int32_t interfaceEntriesRead;
    int32_t fieldEntriesRead;
    int32_t methodEntriesRead;
    int32_t attributeEntriesRead;
    int32_t validityEntriesChecked;
};

int openClassFile(JavaClass*, const char*);
#endif // JAVACLASS_H
