#include <string.h>
#include <inttypes.h>
#include "fileParser.h"
#include "constantPool.h"
#include "utf8.h"

char readCPClass(JavaClass* jc, cp_info* entry) {
    if (!readu2(jc, &entry->Class.name_index)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }
    if (entry->Class.name_index == 0 ||
        entry->Class.name_index >= jc->constantPoolCount) {
        jc->status = INV_CP_INDEX;
        return 0;
    }
    return 1;
}

char readCPFieldref(JavaClass* jc, cp_info* entry) {
    if (!readu2(jc, &entry->Fieldref.class_index)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }

    if (entry->Fieldref.class_index == 0 ||
        entry->Fieldref.class_index >= jc->constantPoolCount) {
        jc->status = INV_CP_INDEX;
        return 0;
    }

    if (!readu2(jc, &entry->Fieldref.name_and_type_index)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }

    if (entry->Fieldref.name_and_type_index == 0 || entry->Fieldref.name_and_type_index >= jc->constantPoolCount) {
        jc->status = INV_CP_INDEX;
        return 0;
    }

    return 1;
}

char readCPInteger(JavaClass* jc, cp_info* entry) {
    if (!readu4(jc, &entry->Integer.value)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }

    return 1;
}

char readCPLong(JavaClass* jc, cp_info* entry) {
    if (!readu4(jc, &entry->Long.high)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }

    if (!readu4(jc, &entry->Long.low)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }

    return 1;
}

char readCPUtf8(JavaClass* jc, cp_info* entry) {
    if (!readu2(jc, &entry->Utf8.length)) {
        jc->status = UNXPTD_EOF_READING_CP;
        return 0;
    }

    if (entry->Utf8.length > 0) {
        entry->Utf8.bytes = (uint8_t*)malloc(entry->Utf8.length);

        if (!entry->Utf8.bytes) {
            jc->status = MEM_ALLOC_FAILED;
            return 0;
        }

        uint16_t i;
        uint8_t* bytes = entry->Utf8.bytes;

        for (i = 0; i < entry->Utf8.length; i++) {
            int byte = fgetc(jc->file);

            if (byte == EOF) {
                jc->status = UNXPTD_EOF_READING_UTF8;
                return 0;
            }

            jc->totalBytesRead++;

            if (byte == 0 || (byte >= 0xF0)) {
                jc->status = INV_UTF8_BYTES;
                return 0;
            }

            *bytes++ = (uint8_t)byte;
        }
    }
    else {
        entry->Utf8.bytes = NULL;
    }

    return 1;
}

char readCPEntry(JavaClass* jc, cp_info* entry) {
    // Gets the entry tag
    int byte = fgetc(jc->file);

    if (byte == EOF) {
        jc->status = UNXPTD_EOF_READING_CP;
        entry->tag = 0xFF;
        return 0;
    }

    entry->tag = (uint8_t)byte;

    jc->totalBytesRead++;
    jc->lastTagRead = entry->tag;

    switch(entry->tag) {
        case CONST_MethodType: // Compatibility with Java 8
        case CONST_Class:
        case CONST_String:
            return readCPClass(jc, entry);

        case CONST_Utf8:
            return readCPUtf8(jc, entry);

        case CONST_InvokeDynamic: // Compatibility with Java 8
        case CONST_Fieldref:
        case CONST_Methodref:
        case CONST_InterfaceMethodref:
        case CONST_NameAndType:
            return readCPFieldref(jc, entry);

        case CONST_Integer:
        case CONST_Float:
            return readCPInteger(jc, entry);

        case CONST_Long:
        case CONST_Double:
            return readCPLong(jc, entry);

        case CONST_MethodHandle:

            if (!readu2(jc, NULL) || fgetc(jc->file) == EOF) {
                jc->status = UNXPTD_EOF_READING_CP;
                return 0;
            }

            jc->totalBytesRead++;

            break;

        default:
            jc->status = UNKNOWN_CP_TAG;
            break;
    }
    return 0;
}

const char* decodeTag(uint8_t tag) {
    switch(tag) {
        case CONST_Class: return "Class";
        case CONST_Double: return "Double";
        case CONST_Fieldref: return "Fieldref";
        case CONST_Float: return "Float";
        case CONST_Integer: return "Integer";
        case CONST_InterfaceMethodref: return "InterfaceMethodref";
        case CONST_Long: return "Long";
        case CONST_Methodref: return "Methodref";
        case CONST_NameAndType: return "NameAndType";
        case CONST_String: return "String";
        case CONST_Utf8: return "Utf8";
        default:
            break;
    }
    return "Unknown Tag";
}

void printCPEntry(JavaClass* jc, cp_info* entry) {
    char buffer[48];
    uint32_t u32;
    cp_info* cpi;

    switch(entry->tag) {
        case CONST_Utf8:
            u32 = UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), entry->Utf8.bytes, entry->Utf8.length);
            printf("\tLength of byte array: %u\n", entry->Utf8.length);
            printf("\tLength of string: %u\n", u32);
            printf("\tString: %s", buffer);
            if (u32 != entry->Utf8.length)
                printf("\n\tUTF-8 Characters: %.*s", (int)entry->Utf8.length, entry->Utf8.bytes);
            break;

        case CONST_String:
            cpi = jc->constantPool + entry->String.string_index - 1;
            u32 = UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tString: #%u %s",entry->String.string_index, buffer);
            if (u32 != cpi->Utf8.length)
                printf("\n\tUTF-8: %.*s", (int)cpi->Utf8.length, cpi->Utf8.bytes);
            break;

        case CONST_Fieldref:
        case CONST_Methodref:
        case CONST_InterfaceMethodref:
            cpi = jc->constantPool + entry->Fieldref.class_index - 1;
            cpi = jc->constantPool + cpi->Class.name_index - 1;
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tClass name: #%u <%s>\n", entry->Fieldref.class_index, buffer);
            cpi = jc->constantPool + entry->Fieldref.name_and_type_index - 1;
            u32 = cpi->NameAndType.name_index;
            cpi = jc->constantPool + u32 - 1;
            printf("\tName and type: #%u <%s :", entry->Fieldref.name_and_type_index, buffer);
            cpi = jc->constantPool + entry->Fieldref.name_and_type_index - 1;
            u32 = cpi->NameAndType.descriptor_index;
            cpi = jc->constantPool + u32 - 1;
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf(" %s>", buffer);
            break;

        case CONST_Class:
            cpi = jc->constantPool + entry->Class.name_index - 1;
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tClass name: #%u <%s>", entry->Class.name_index, buffer);
            break;

        case CONST_NameAndType:
            cpi = jc->constantPool + entry->NameAndType.name_index - 1;
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tName: #%u <%s>\n", entry->NameAndType.name_index, buffer);
            cpi = jc->constantPool + entry->NameAndType.descriptor_index - 1;
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tDescriptor: #%u <%s>", entry->NameAndType.descriptor_index, buffer);
            break;

        case CONST_Integer:
            printf("\tBytes: 0x%08X\n", entry->Integer.value);
            printf("\tInteger: %d", (int32_t)entry->Integer.value);
            break;

        case CONST_Long:
            printf("\tHigh bytes: 0x%08X\n", entry->Long.high);
            printf("\tLow  bytes: 0x%08X\n", entry->Long.low);
            printf("\tLong: %" PRId64"", ((int64_t)entry->Long.high << 32) | entry->Long.low);
            break;

        case CONST_Float:
            printf("\tBytes: 0x%08X\n", entry->Float.bytes);
            printf("\tFloat: %e", readFloatFromUint32(entry->Float.bytes));
            break;

        case CONST_Double:
            printf("\tHigh bytes:   0x%08X\n", entry->Double.high);
            printf("\tLow  bytes:   0x%08X\n", entry->Double.low);
            printf("\tDouble: %e", readDoubleFromUint64((uint64_t)entry->Double.high << 32 | entry->Double.low));
            break;

        default:
            break;
    }
    printf("\n");
}

void printCP(JavaClass* jc) {
    uint16_t u16;
    cp_info* cp;

    if (jc->constantPoolCount > 1) {
        printf("\n==== Constant Pool ====\n");

        for (u16 = 0; u16 < jc->constantPoolCount - 1; u16++) {
            cp = jc->constantPool + u16;
            printf("\n[%u]: CONSTANT_%s\n", u16 + 1, decodeTag(cp->tag));
            printCPEntry(jc, cp);

            if (cp->tag == CONST_Double || cp->tag == CONST_Long)
                u16++;
        }
    }
}
