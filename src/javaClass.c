#include "javaClass.h"

int openClassFile(JavaClass* jc, const char* path) {
    if (!jc)    return 1;

    uint32_t u32;
    uint16_t u16;

    jc->file = fopen(path, "rb");
    jc->minorVersion = jc->majorVersion = jc->constantPoolCount = 0;
    jc->constantPool = NULL;
    jc->interfaces = NULL;
    jc->fields = NULL;
    jc->methods = NULL;
    jc->attributes = NULL;
    jc->status = CLASS_STATUS_OK;
    jc->classNameMismatch = 0;

    jc->thisClass = jc->superClass = jc->accessFlags = 0;
    jc->attributeCount = jc->fieldCount = jc->methodCount = jc->constantPoolCount = jc->interfaceCount = 0;

    jc->staticFieldCount = 0;
    jc->instanceFieldCount = 0;

    jc->lastTagRead = 0;
    jc->totalBytesRead = 0;
    jc->constantPoolEntriesRead = 0;
    jc->attributeEntriesRead = 0;
    jc->constantPoolEntriesRead = 0;
    jc->interfaceEntriesRead = 0;
    jc->fieldEntriesRead = 0;
    jc->methodEntriesRead = 0;
    jc->validityEntriesChecked = 0;

    if (!jc->file) {
        jc->status = CLASS_STATUS_FILE_COULDNT_BE_OPENED;
        return 1;
    }

    if (!readu4(jc, &u32) || u32 != 0xCAFEBABE) {
        jc->status = CLASS_STATUS_INVALID_SIGNATURE;
        return 1;
    }

    if (!readu2(jc, &jc->minorVersion) || !readu2(jc, &jc->majorVersion) ||
        !readu2(jc, &jc->constantPoolCount)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (jc->majorVersion < 45 || jc->majorVersion > 52) {
        jc->status = CLASS_STATUS_UNSUPPORTED_VERSION;
        return 1;
    }

    if (jc->constantPoolCount == 0) {
        jc->status = INVALID_CONSTANT_POOL_COUNT;
        return 1;
    }

    if (jc->constantPoolCount > 1) {
        jc->constantPool = (cp_info*)malloc(sizeof(cp_info) * (jc->constantPoolCount - 1));

        if (!jc->constantPool) {
            jc->status = MEMORY_ALLOCATION_FAILED;
            return 1;
        }

        for (u16 = 0; u16 < jc->constantPoolCount - 1; u16++) {
            if (!readConstantPoolEntry(jc, jc->constantPool + u16)) {
                jc->constantPoolCount = u16 + 1;
                return;
            }

            if (jc->constantPool[u16].tag == CONSTANT_Double ||
                jc->constantPool[u16].tag == CONSTANT_Long) {
                u16++;
            }
            jc->constantPoolEntriesRead++;
        }

        if (!checkConstantPoolValidity(jc))
            return 1;
    }

    if (!readu2(jc, &jc->accessFlags) || !readu2(jc, &jc->thisClass) ||
        !readu2(jc, &jc->superClass)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (!checkClassIndexAndAccessFlags(jc))
        return 1;

    if (!checkClassNameFileNameMatch(jc, path))
        jc->classNameMismatch = 1;

    if (!readu2(jc, &jc->interfaceCount)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (jc->interfaceCount > 0) {
        jc->interfaces = (uint16_t*)malloc(sizeof(uint16_t) * jc->interfaceCount);

        if (!jc->interfaces) {
            jc->status = MEMORY_ALLOCATION_FAILED;
            return 1;
        }

        for (u32 = 0; u32 < jc->interfaceCount; u32++) {
            if (!readu2(jc, &u16)) {
                jc->status = UNEXPECTED_EOF_READING_INTERFACES;
                return 1;
            }

            if (u16 == 0 || jc->constantPool[u16 - 1].tag != CONSTANT_Class) {
                jc->status = INVALID_INTERFACE_INDEX;
                return 1;
            }

            *(jc->interfaces + u32) = u16;
            jc->interfaceEntriesRead++;
        }
    }

    if (!readu2(jc, &jc->fieldCount)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (jc->fieldCount > 0) {
        jc->fields = (field_info*)malloc(sizeof(field_info) * jc->fieldCount);

        if (!jc->fields) {
            jc->status = MEMORY_ALLOCATION_FAILED;
            return 1;
        }

        for (u32 = 0; u32 < jc->fieldCount; u32++) {
            field_info* field = jc->fields + u32;
            uint8_t isCat2;

            if (!readField(jc, field)) {
                jc->fieldCount = u32 + 1;
                return 1;
            }

            isCat2 = *jc->constantPool[field->descriptor_index - 1].Utf8.bytes;
            isCat2 = isCat2 == 'J' || isCat2 == 'D';

            if (field->access_flags & ACC_STATIC) {
                field->offset = jc->staticFieldCount++;
                jc->staticFieldCount += isCat2;
            }
            else {
                field->offset = jc->instanceFieldCount++;
                jc->instanceFieldCount += isCat2;
            }

            jc->fieldEntriesRead++;
        }
    }

    if (!readu2(jc, &jc->methodCount)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (jc->methodCount > 0) {
        jc->methods = (method_info*)malloc(sizeof(method_info) * jc->methodCount);

        if (!jc->methods) {
            jc->status = MEMORY_ALLOCATION_FAILED;
            return 1;
        }

        for (u32 = 0; u32 < jc->methodCount; u32++) {
            if (!readMethod(jc, jc->methods + u32)) {
                jc->methodCount = u32 + 1;
                return 1;
            }

            jc->methodEntriesRead++;
        }
    }

    if (!readu2(jc, &jc->attributeCount)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (jc->attributeCount > 0) {
        jc->attributeEntriesRead = 0;
        jc->attributes = (attribute_info*)malloc(sizeof(attribute_info) * jc->attributeCount);

        if (!jc->attributes) {
            jc->status = MEMORY_ALLOCATION_FAILED;
            return 1;
        }

        for (u32 = 0; u32 < jc->attributeCount; u32++) {
            if (!readAttribute(jc, jc->attributes + u32)) {
                jc->attributeCount = u32 + 1;
                return 1;
            }

            jc->attributeEntriesRead++;
        }
    }

    if (fgetc(jc->file) != EOF) {
        jc->status = FILE_CONTAINS_UNEXPECTED_DATA;
    }
    else {
        fclose(jc->file);
        jc->file = NULL;
    }
    return 0;
}
