#include "javaClass.h"
#include "fileParser.h"
#include "validity.h"
#include "constantPool.h"
#include "utf8.h"

int openClassFile(JavaClass* jc, const char* path) {
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
                return 1;
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

    if (!readu2(jc, &jc->interfaceCount)) {
        jc->status = UNEXPECTED_EOF;
        return 1;
    }

    if (jc->interfaceCount > 0) {
        jc->interfaces = (uint16_t*)malloc(sizeof(uint16_t) * jc->interfaceCount);

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

void decodeAccessFlags(uint16_t flags, char* buffer, int32_t buffer_len, enum AccessFlagsType acctype) {
    uint32_t bytes = 0;
    char* buffer_ptr = buffer;
    const char* comma = ", ";
    const char* empty = "";

    #define DECODE_FLAG(flag, name) if (flags & flag) { \
        bytes = snprintf(buffer, buffer_len, "%s%s", bytes ? comma : empty, name); \
        buffer += bytes; \
        buffer_len -= bytes; }

    if (acctype == ACCT_CLASS)
        DECODE_FLAG(ACC_SUPER, "super")

    if (acctype == ACCT_CLASS || acctype == ACCT_METHOD || acctype == ACCT_INNERCLASS)
        DECODE_FLAG(ACC_ABSTRACT, "abstract")

    if (acctype == ACCT_METHOD) {
        DECODE_FLAG(ACC_SYNCHRONIZED, "synchronized")
        DECODE_FLAG(ACC_NATIVE, "native")
        DECODE_FLAG(ACC_STRICT, "strict")
    }

    if (acctype == ACCT_FIELD) {
        DECODE_FLAG(ACC_TRANSIENT, "transient")
        DECODE_FLAG(ACC_VOLATILE, "volatile")
    }

    DECODE_FLAG(ACC_PUBLIC, "public")

    if (acctype == ACCT_FIELD || acctype == ACCT_METHOD || acctype == ACCT_INNERCLASS) {
        DECODE_FLAG(ACC_PRIVATE, "private")
        DECODE_FLAG(ACC_PROTECTED, "protected")
    }

    if (acctype == ACCT_FIELD || acctype == ACCT_METHOD || acctype == ACCT_INNERCLASS)
        DECODE_FLAG(ACC_STATIC, "static")

    DECODE_FLAG(ACC_FINAL, "final")

    if (acctype == ACCT_CLASS || acctype == ACCT_INNERCLASS)
        DECODE_FLAG(ACC_INTERFACE, "interface")

    if (buffer == buffer_ptr)
        snprintf(buffer, buffer_len, "no flags");

    #undef DECODE_FLAG
}

void printClassFileInfo(JavaClass* jc) {
    char buffer[256];
    cp_info* cp;
    uint16_t u16;

    if (jc->classNameMismatch)
        printf("---- Warning ----\n\nClass name and file path don't match.\nReading will proceed anyway.\n\n");

    printf("---- General Information ----\n\n");

    printf("Version:\t\t%u.%u (Major.Minor)", jc->majorVersion, jc->minorVersion);

    if (jc->majorVersion >= 45 && jc->majorVersion <= 52)
        printf(" [jdk version 1.%d]", jc->majorVersion - 44);

    printf("\nConstant pool count:\t%u\n", jc->constantPoolCount);

    decodeAccessFlags(jc->accessFlags, buffer, sizeof(buffer), ACCT_CLASS);
    printf("Access flags:\t\t0x%.4X [%s]\n", jc->accessFlags, buffer);

    cp = jc->constantPool + jc->thisClass - 1;
    cp = jc->constantPool + cp->Class.name_index - 1;
    UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cp->Utf8.bytes, cp->Utf8.length);
    printf("This class:\t\tcp index #%u <%s>\n", jc->thisClass, buffer);

    cp = jc->constantPool + jc->superClass - 1;
    cp = jc->constantPool + cp->Class.name_index - 1;
    UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cp->Utf8.bytes, cp->Utf8.length);
    printf("Super class:\t\tcp index #%u <%s>\n", jc->superClass, buffer);

    printf("Interfaces count:\t%u\n", jc->interfaceCount);
    printf("Fields count:\t\t%u\n", jc->fieldCount);
    printf("Methods count:\t\t%u\n", jc->methodCount);
    printf("Attributes count:\t%u\n", jc->attributeCount);

    printConstantPool(jc);

    if (jc->interfaceCount > 0) {
        printf("\n---- Interfaces implemented by the class ----\n\n");

        for (u16 = 0; u16 < jc->interfaceCount; u16++) {
            cp = jc->constantPool + *(jc->interfaces + u16) - 1;
            cp = jc->constantPool + cp->Class.name_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cp->Utf8.bytes, cp->Utf8.length);
            printf("\tInterface #%u: #%u <%s>\n", u16 + 1, *(jc->interfaces + u16), buffer);
        }
    }

    printAllFields(jc);
    printMethods(jc);
    printAllAttributes(jc);
}

void closeClassFile(JavaClass* jc) {
    if (!jc)
        return;

    uint16_t i;

    if (jc->file) {
        fclose(jc->file);
        jc->file = NULL;
    }

    if (jc->interfaces) {
        free(jc->interfaces);
        jc->interfaces = NULL;
        jc->interfaceCount = 0;
    }

    if (jc->constantPool) {
        for (i = 0; i < jc->constantPoolCount - 1; i++) {
            if (jc->constantPool[i].tag == CONSTANT_Utf8 && jc->constantPool[i].Utf8.bytes)
                free(jc->constantPool[i].Utf8.bytes);
        }

        free(jc->constantPool);
        jc->constantPool = NULL;
        jc->constantPoolCount = 0;
    }

    if (jc->methods) {
        for (i = 0; i < jc->methodCount; i++)
            freeMethodAttributes(jc->methods + i);
        free(jc->methods);
        jc->methodCount = 0;
    }

    if (jc->fields) {
        for (i = 0; i < jc->fieldCount; i++)
            freeFieldAttributes(jc->fields + i);
        free(jc->fields);
        jc->fieldCount = 0;
    }

    if (jc->attributes) {
        for (i = 0; i < jc->attributeCount; i++)
            freeAttributeInfo(jc->attributes + i);

        free(jc->attributes);
        jc->attributeCount = 0;
    }
}
