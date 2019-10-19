#include <string.h>
#include <inttypes.h>
#include "fileParser.h"
#include "constantPool.h"
#include "utf8.h"

char readConstantPool_Class(JavaClass* jc, cp_info* entry) {
    if (!readu2(jc, &entry->Class.name_index)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    if (entry->Class.name_index == 0 ||
        entry->Class.name_index >= jc->constantPoolCount) {
        jc->status = INVALID_CONSTANT_POOL_INDEX;
        return 0;
    }
    return 1;
}

char readConstantPool_Fieldref(JavaClass* jc, cp_info* entry) {
    if (!readu2(jc, &entry->Fieldref.class_index)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    if (entry->Fieldref.class_index == 0 ||
        entry->Fieldref.class_index >= jc->constantPoolCount) {
        jc->status = INVALID_CONSTANT_POOL_INDEX;
        return 0;
    }

    if (!readu2(jc, &entry->Fieldref.name_and_type_index)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    if (entry->Fieldref.name_and_type_index == 0 ||
        entry->Fieldref.name_and_type_index >= jc->constantPoolCount) {
        jc->status = INVALID_CONSTANT_POOL_INDEX;
        return 0;
    }

    return 1;
}

char readConstantPool_Integer(JavaClass* jc, cp_info* entry) {
    if (!readu4(jc, &entry->Integer.value)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    return 1;
}

char readConstantPool_Long(JavaClass* jc, cp_info* entry) {
    if (!readu4(jc, &entry->Long.high)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    if (!readu4(jc, &entry->Long.low)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    return 1;
}

char readConstantPool_Utf8(JavaClass* jc, cp_info* entry) {
    if (!readu2(jc, &entry->Utf8.length)) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        return 0;
    }

    if (entry->Utf8.length > 0) {
        entry->Utf8.bytes = (uint8_t*)malloc(entry->Utf8.length);

        if (!entry->Utf8.bytes) {
            jc->status = MEMORY_ALLOCATION_FAILED;
            return 0;
        }

        uint16_t i;
        uint8_t* bytes = entry->Utf8.bytes;

        for (i = 0; i < entry->Utf8.length; i++) {
            int byte = fgetc(jc->file);

            if (byte == EOF) {
                jc->status = UNEXPECTED_EOF_READING_UTF8;
                return 0;
            }

            jc->totalBytesRead++;

            if (byte == 0 || (byte >= 0xF0)) {
                jc->status = INVALID_UTF8_BYTES;
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

char readConstantPoolEntry(JavaClass* jc, cp_info* entry) {
    // Gets the entry tag
    int byte = fgetc(jc->file);

    if (byte == EOF) {
        jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
        entry->tag = 0xFF;
        return 0;
    }

    entry->tag = (uint8_t)byte;

    jc->totalBytesRead++;
    jc->lastTagRead = entry->tag;

    switch(entry->tag) {
        case CONSTANT_MethodType: // Compatibility with Java 8
        case CONSTANT_Class:
        case CONSTANT_String:
            return readConstantPool_Class(jc, entry);

        case CONSTANT_Utf8:
            return readConstantPool_Utf8(jc, entry);

        case CONSTANT_InvokeDynamic: // Compatibility with Java 8
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_NameAndType:
            return readConstantPool_Fieldref(jc, entry);

        case CONSTANT_Integer:
        case CONSTANT_Float:
            return readConstantPool_Integer(jc, entry);

        case CONSTANT_Long:
        case CONSTANT_Double:
            return readConstantPool_Long(jc, entry);

        case CONSTANT_MethodHandle:

            if (!readu2(jc, NULL) || fgetc(jc->file) == EOF) {
                jc->status = UNEXPECTED_EOF_READING_CONSTANT_POOL;
                return 0;
            }

            jc->totalBytesRead++;

            break;

        default:
            jc->status = UNKNOWN_CONSTANT_POOL_TAG;
            break;
    }
    return 0;
}

const char* decodeTag(uint8_t tag) {
    switch(tag) {
        case CONSTANT_Class: return "Class";
        case CONSTANT_Double: return "Double";
        case CONSTANT_Fieldref: return "Fieldref";
        case CONSTANT_Float: return "Float";
        case CONSTANT_Integer: return "Integer";
        case CONSTANT_InterfaceMethodref: return "InterfaceMethodref";
        case CONSTANT_Long: return "Long";
        case CONSTANT_Methodref: return "Methodref";
        case CONSTANT_NameAndType: return "NameAndType";
        case CONSTANT_String: return "String";
        case CONSTANT_Utf8: return "Utf8";
        default:
            break;
    }
    return "Unknown Tag";
}

void printConstantPoolEntry(JavaClass* jc, cp_info* entry) {
    char buffer[48];
    uint32_t u32;
    cp_info* cpi;

    switch(entry->tag) {
        case CONSTANT_Utf8:
            u32 = UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), entry->Utf8.bytes, entry->Utf8.length);
            printf("\tLength of byte array: %u\n", entry->Utf8.length);
            printf("\tLength of string: %u\n", u32);
            printf("\tString: %s", buffer);
            if (u32 != entry->Utf8.length)
                printf("\n\tUTF-8 Characters: %.*s", (int)entry->Utf8.length, entry->Utf8.bytes);
            break;

        case CONSTANT_String:
            cpi = jc->constantPool + entry->String.string_index - 1;
            u32 = UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tString: #%u %s",entry->String.string_index, buffer);
            if (u32 != cpi->Utf8.length)
                printf("\n\tUTF-8: %.*s", (int)cpi->Utf8.length, cpi->Utf8.bytes);
            break;

        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            cpi = jc->constantPool + entry->Fieldref.class_index - 1;
            cpi = jc->constantPool + cpi->Class.name_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tClass name: #%u <%s>\n", entry->Fieldref.class_index, buffer);
            cpi = jc->constantPool + entry->Fieldref.name_and_type_index - 1;
            u32 = cpi->NameAndType.name_index;
            cpi = jc->constantPool + u32 - 1;
            printf("\tName and type: #%u <%s :", entry->Fieldref.name_and_type_index, buffer);
            cpi = jc->constantPool + entry->Fieldref.name_and_type_index - 1;
            u32 = cpi->NameAndType.descriptor_index;
            cpi = jc->constantPool + u32 - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf(" %s>", buffer);
            break;

        case CONSTANT_Class:
            cpi = jc->constantPool + entry->Class.name_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tClass name: #%u <%s>", entry->Class.name_index, buffer);
            break;

        case CONSTANT_NameAndType:
            cpi = jc->constantPool + entry->NameAndType.name_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tName: #%u <%s>\n", entry->NameAndType.name_index, buffer);
            cpi = jc->constantPool + entry->NameAndType.descriptor_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tDescriptor: #%u <%s>", entry->NameAndType.descriptor_index, buffer);
            break;

        case CONSTANT_Integer:
            printf("\tBytes: 0x%08X\n", entry->Integer.value);
            printf("\tInteger: %d", (int32_t)entry->Integer.value);
            break;

        case CONSTANT_Long:
            printf("\tHigh bytes: 0x%08X\n", entry->Long.high);
            printf("\tLow  bytes: 0x%08X\n", entry->Long.low);
            printf("\tLong: %" PRId64"", ((int64_t)entry->Long.high << 32) | entry->Long.low);
            break;

        case CONSTANT_Float:
            printf("\tBytes: 0x%08X\n", entry->Float.bytes);
            printf("\tFloat: %e", readFloatFromUint32(entry->Float.bytes));
            break;

        case CONSTANT_Double:
            printf("\tHigh bytes:   0x%08X\n", entry->Double.high);
            printf("\tLow  bytes:   0x%08X\n", entry->Double.low);
            printf("\tDouble: %e", readDoubleFromUint64((uint64_t)entry->Double.high << 32 | entry->Double.low));
            break;

        default:
            break;
    }
    printf("\n");
}

void printConstantPool(JavaClass* jc) {
    uint16_t u16;
    cp_info* cp;

    if (jc->constantPoolCount > 1) {
        printf("\n==== Constant Pool ====\n");

        for (u16 = 0; u16 < jc->constantPoolCount - 1; u16++) {
            cp = jc->constantPool + u16;
            printf("\n[%u]: CONSTANT_%s (tag = %u)\n", u16 + 1, decodeTag(cp->tag), cp->tag);
            printConstantPoolEntry(jc, cp);

            if (cp->tag == CONSTANT_Double || cp->tag == CONSTANT_Long)
                u16++;
        }
    }
}
