#include "methods.h"
#include "fileParser.h"
#include "validity.h"
#include "utf8.h"
#include "string.h"

char readMethod(JavaClass* jc, method_info* entry) {
    entry->attributes = NULL;
    jc->attributeEntriesRead = -1;

    if (!readu2(jc, &entry->access_flags) || !readu2(jc, &entry->name_index) ||
        !readu2(jc, &entry->descriptor_index) || !readu2(jc, &entry->attributes_count)) {
        jc->status = UNEXPECTED_EOF;
        return 0;
    }

    if (!checkMethodAccessFlags(jc, entry->access_flags))
        return 0;

    if (entry->name_index == 0 || entry->name_index >= jc->constantPoolCount ||
        !isValidMethodNameIndex(jc, entry->name_index)) {
        jc->status = INVALID_NAME_INDEX;
        return 0;
    }

    cp_info* cpi = jc->constantPool + entry->descriptor_index - 1;

    if (entry->descriptor_index == 0 || entry->descriptor_index >= jc->constantPoolCount ||
        cpi->tag != CONSTANT_Utf8 ||
        cpi->Utf8.length != readMethodDescriptor(cpi->Utf8.bytes, cpi->Utf8.length, 1)) {
        jc->status = INVALID_FIELD_DESCRIPTOR_INDEX;
        return 0;
    }

    if (entry->attributes_count > 0) {
        entry->attributes = (attribute_info*)malloc(sizeof(attribute_info) * entry->attributes_count);
        if (!entry->attributes) {
            jc->status = MEM_ALLOC_FAILED;
            return 0;
        }

        uint16_t i;

        jc->attributeEntriesRead = 0;

        for (i = 0; i < entry->attributes_count; i++) {
            if (!readAttribute(jc, entry->attributes + i)) {
                entry->attributes_count = i + 1;
                return 0;
            }
            jc->attributeEntriesRead++;
        }
    }
    return 1;
}

void freeMethodAttributes(method_info* entry) {
    uint32_t i;

    if (entry->attributes != NULL) {
        for (i = 0; i < entry->attributes_count; i++)
            freeAttributeInfo(entry->attributes + i);

        free(entry->attributes);

        entry->attributes_count = 0;
        entry->attributes = NULL;
    }
}

void printMethods(JavaClass* jc) {
    uint16_t u16, att_index;
    char buffer[48];
    cp_info* cpi;
    method_info* mi;
    attribute_info* atti;

    if (jc->methodCount > 0) {
        printf("\n==== Methods ====\n");
        for (u16 = 0; u16 < jc->methodCount; u16++) {
            mi = jc->methods + u16;

            cpi = jc->constantPool + mi->name_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);

            printf("\nMethod #%u:\n\n", u16 + 1);
            printf("\tname_index:        #%u <%s>\n", mi->name_index, buffer);
            cpi = jc->constantPool + mi->descriptor_index - 1;
            UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("\tdescriptor_index:  #%u <%s>\n", mi->descriptor_index, buffer);
            decodeAccessFlags(mi->access_flags, buffer, sizeof(buffer), ACCT_METHOD);
            printf("\taccess_flags:      0x%.4X [%s]\n", mi->access_flags, buffer);
            printf("\tattribute_count:   %u\n", mi->attributes_count);

            if (mi->attributes_count > 0) {
                for (att_index = 0; att_index < mi->attributes_count; att_index++) {
                    atti = mi->attributes + att_index;
                    cpi = jc->constantPool + atti->name_index - 1;
                    UTF8_to_Ascii((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);

                    printf("\n\tMethod Attribute #%u - %s:\n", att_index + 1, buffer);
                    printAttribute(jc, atti, 2);
                }
            }
        }
    }
}

method_info* getMethodMatching(JavaClass* jc, const uint8_t* name, int32_t name_len, const uint8_t* descriptor,
                               int32_t descriptor_len, uint16_t flag_mask) {
    method_info* method = jc->methods;
    cp_info* cpi;
    uint16_t index;

    for (index = jc->methodCount; index > 0; index--, method++) {
        if ((method->access_flags & flag_mask) != flag_mask)
            continue;

        cpi = jc->constantPool + method->name_index - 1;
        if (!cmp_UTF8_Ascii(cpi->Utf8.bytes, cpi->Utf8.length, name, name_len))
            continue;

        cpi = jc->constantPool + method->descriptor_index - 1;
        if (!cmp_UTF8_Ascii(cpi->Utf8.bytes, cpi->Utf8.length, descriptor, descriptor_len))
            continue;

        return method;
    }
    return NULL;
}
