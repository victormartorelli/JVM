#include "validity.h"
#include "constantPool.h"
#include "utf8.h"
#include "fileParser.h"
#include <locale.h>
#include <wctype.h>
#include <ctype.h>

char checkMethodAccessFlags(JavaClass* jc, uint16_t accessFlags) {
    if (accessFlags & ACC_INVALID_METHOD_FLAG_MASK) {
        jc->status = USE_OF_RSVD_METHOD_ACCESS_FLAGS;
        return 0;
    }

    if ((accessFlags & ACC_ABSTRACT) &&
        (accessFlags & (ACC_FINAL | ACC_NATIVE | ACC_PRIVATE | ACC_STATIC | ACC_STRICT | ACC_SYNCHRONIZED))) {
        jc->status = INV_ACCESS_FLAGS;
        return 0;
    }

    uint8_t accessModifierCount = 0;

    if (accessFlags & ACC_PUBLIC)
        accessModifierCount++;

    if (accessFlags & ACC_PRIVATE)
        accessModifierCount++;

    if (accessFlags & ACC_PROTECTED)
        accessModifierCount++;

    if (accessModifierCount > 1) {
        jc->status = INV_ACCESS_FLAGS;
        return 0;
    }
    return 1;
}

char checkFieldAccessFlags(JavaClass* jc, uint16_t accessFlags) {
    if (accessFlags & ACC_INVALID_FIELD_FLAG_MASK) {
        jc->status = USE_OF_RSVD_FIELD_ACCESS_FLAGS;
        return 0;
    }

    if ((accessFlags & ACC_ABSTRACT) && (accessFlags & ACC_FINAL)) {
        jc->status = INV_ACCESS_FLAGS;
        return 0;
    }

    uint16_t interfaceRequiredBitMask = ACC_PUBLIC | ACC_STATIC | ACC_FINAL;

    if ((jc->accessFlags & ACC_INTERFACE) &&
        ((accessFlags & interfaceRequiredBitMask) != interfaceRequiredBitMask)) {
        jc->status = INV_ACCESS_FLAGS;
        return 0;
    }

    uint8_t accessModifierCount = 0;

    if (accessFlags & ACC_PUBLIC)
        accessModifierCount++;

    if (accessFlags & ACC_PRIVATE)
        accessModifierCount++;

    if (accessFlags & ACC_PROTECTED)
        accessModifierCount++;

    if (accessModifierCount > 1) {
        jc->status = INV_ACCESS_FLAGS;
        return 0;
    }

    return 1;
}

char checkClassIndexAndAccessFlags(JavaClass* jc) {
    if (jc->accessFlags & ACC_INVALID_CLASS_FLAG_MASK) {
        jc->status = USE_OF_RSVD_CLASS_ACCESS_FLAGS;
        return 0;
    }

    if (jc->accessFlags & ACC_INTERFACE) {
        if ((jc->accessFlags & ACC_ABSTRACT) == 0 ||
            (jc->accessFlags & (ACC_FINAL | ACC_SUPER))) {
            jc->status = INV_ACCESS_FLAGS;
            return 0;
        }
    }

    if (!jc->thisClass || jc->thisClass >= jc->constantPoolCount ||
        jc->constantPool[jc->thisClass - 1].tag != CONSTANT_Class) {
        jc->status = INV_THIS_CLASS_IDX;
        return 0;
    }

    if (jc->superClass >= jc->constantPoolCount ||
        (jc->superClass && jc->constantPool[jc->superClass - 1].tag != CONSTANT_Class)) {
        jc->status = INV_SUPER_CLASS_IDX;
        return 0;
    }
    return 1;
}

char checkClassNameFileNameMatch(JavaClass* jc, const char* classFilePath) {
    int32_t i, begin = 0, end;
    cp_info* cpi;

    for (i = 0; classFilePath[i] != '\0'; i++) {
        if (classFilePath[i] == '/' || classFilePath[i] == '\\')
            begin = i + 1;
        else if (classFilePath[i] == '.')
            break;
    }

    end = i;

    cpi = jc->constantPool + jc->thisClass - 1;
    cpi = jc->constantPool + cpi->Class.name_index - 1;

    for (i = 0; i < cpi->Utf8.length; i++) {
        if (*(cpi->Utf8.bytes + i) == '/') {
            if (begin == 0)
                break;

            while (--begin > 0 && (classFilePath[begin - 1] != '/' || classFilePath[begin - 1] != '\\'));
        }
    }

    return cmp_UTF8_FilePath(cpi->Utf8.bytes, cpi->Utf8.length, (uint8_t*)classFilePath + begin, end - begin);
}

char isValidJavaIdentifier(uint8_t* utf8_bytes, int32_t utf8_len, uint8_t isClassIdentifier) {
    uint32_t utf8_char;
    uint8_t used_bytes;
    uint8_t firstChar = 1;
    char isValid = 1;

    if (*utf8_bytes == '[')
        return readFieldDescriptor(utf8_bytes, utf8_len, 1) == utf8_len;

    while (utf8_len > 0) {
        used_bytes = nextUTF8Character(utf8_bytes, utf8_len, &utf8_char);

        if (used_bytes == 0) {
            isValid = 0;
            break;
        }

        if (isalpha(utf8_char) || utf8_char == '_' || utf8_char == '$' ||
            (isdigit(utf8_char) && !firstChar) || iswalpha(utf8_char) ||
            (utf8_char == '/' && !firstChar && isClassIdentifier)) {
            firstChar = utf8_char == '/';
            utf8_len -= used_bytes;
            utf8_bytes += used_bytes;
        }
        else {
            isValid = 0;
            break;
        }
    }
    return isValid;
}

char isValidUTF8Index(JavaClass* jc, uint16_t index) {
    return (jc->constantPool + index - 1)->tag == CONSTANT_Utf8;
}


char isValidNameIndex(JavaClass* jc, uint16_t name_index, uint8_t isClassIdentifier) {
    cp_info* entry = jc->constantPool + name_index - 1;
    return entry->tag == CONSTANT_Utf8 && isValidJavaIdentifier(entry->Utf8.bytes, entry->Utf8.length, isClassIdentifier);
}

char isValidMethodNameIndex(JavaClass* jc, uint16_t name_index) {
    cp_info* entry = jc->constantPool + name_index - 1;

    if (entry->tag != CONSTANT_Utf8)
        return 0;

    if (cmp_UTF8_Ascii(entry->Utf8.bytes, entry->Utf8.length, (uint8_t*)"<init>", 6) ||
        cmp_UTF8_Ascii(entry->Utf8.bytes, entry->Utf8.length, (uint8_t*)"<clinit>", 8))
        return 1;

    return isValidJavaIdentifier(entry->Utf8.bytes, entry->Utf8.length, 0);
}

char checkClassIndex(JavaClass* jc, uint16_t class_index) {
    cp_info* entry = jc->constantPool + class_index - 1;

    if (entry->tag != CONSTANT_Class) {
        jc->status = INV_CLASS_IDX;
        return 0;
    }
    return 1;
}

char checkFieldNameAndTypeIndex(JavaClass* jc, uint16_t name_and_type_index) {
    cp_info* entry = jc->constantPool + name_and_type_index - 1;

    if (entry->tag != CONSTANT_NameAndType || !isValidNameIndex(jc, entry->NameAndType.name_index, 0)) {
        jc->status = INV_NAME_IDX;
        return 0;
    }

    entry = jc->constantPool + entry->NameAndType.descriptor_index - 1;

    if (entry->tag != CONSTANT_Utf8 || entry->Utf8.length == 0) {
        jc->status = INV_FIELD_DESC_IDX;
        return 0;
    }

    if (entry->Utf8.length != readFieldDescriptor(entry->Utf8.bytes, entry->Utf8.length, 1)) {
        jc->status = INV_FIELD_DESC_IDX;
        return 0;
    }

    return 1;
}

char checkMethodNameAndTypeIndex(JavaClass* jc, uint16_t name_and_type_index) {
    cp_info* entry = jc->constantPool + name_and_type_index - 1;

    if (entry->tag != CONSTANT_NameAndType || !isValidMethodNameIndex(jc, entry->NameAndType.name_index)) {
        jc->status = INV_NAME_IDX;
        return 0;
    }

    entry = jc->constantPool + entry->NameAndType.descriptor_index - 1;

    if (entry->tag != CONSTANT_Utf8 || entry->Utf8.length == 0) {
        jc->status = INV_METHOD_DESC_IDX;
        return 0;
    }

    if (entry->Utf8.length != readMethodDescriptor(entry->Utf8.bytes, entry->Utf8.length, 1)) {
        jc->status = INV_METHOD_DESC_IDX;
        return 0;
    }
    return 1;
}


char checkConstantPoolValidity(JavaClass* jc) {
    uint16_t i;
    char success = 1;
    char* previousLocale = setlocale(LC_CTYPE, "pt_BR.UTF-8");

    for (i = 0; success && i < jc->constantPoolCount - 1; i++) {
        cp_info* entry = jc->constantPool + i;

        switch(entry->tag) {
            case CONSTANT_Class:

                if (!isValidNameIndex(jc, entry->Class.name_index, 1)) {
                    jc->status = INV_NAME_IDX;
                    success = 0;
                }

                break;

            case CONSTANT_String:

                if (!isValidUTF8Index(jc, entry->String.string_index)) {
                    jc->status = INV_STRING_IDX;
                    success = 0;
                }

                break;

            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:

                if (!checkClassIndex(jc, entry->Methodref.class_index) ||
                    !checkMethodNameAndTypeIndex(jc, entry->Methodref.name_and_type_index))
                    success = 0;

                break;

            case CONSTANT_Fieldref:

                if (!checkClassIndex(jc, entry->Fieldref.class_index) ||
                    !checkFieldNameAndTypeIndex(jc, entry->Fieldref.name_and_type_index))
                    success = 0;

                break;

            case CONSTANT_NameAndType:

                if (!isValidUTF8Index(jc, entry->NameAndType.name_index) ||
                    !isValidUTF8Index(jc, entry->NameAndType.descriptor_index)) {
                    jc->status = INV_NAME_AND_TYPE_IDX;
                    success = 0;
                }

                break;

            case CONSTANT_Double:
            case CONSTANT_Long:
                i++;
                break;

            case CONSTANT_Float:
            case CONSTANT_Integer:
            case CONSTANT_Utf8:
                break;

            default:
                break;
        }

        jc->validityEntriesChecked = i + 1;
    }

    setlocale(LC_CTYPE, previousLocale);

    return success;
}
