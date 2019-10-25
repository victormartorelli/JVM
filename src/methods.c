#include "string.h"
#include "methods.h"
#include "fileParser.h"
#include "utf8.h"
#include "validity.h"

char readMethod(JavaClass* jc, method_info* entry) {
    entry->attributes = NULL;
    jc->attributeEntriesRead = -1;

    if (!readu2(jc, &entry->access_flags) || !readu2(jc, &entry->name_index) || !readu2(jc, &entry->descriptor_index) || !readu2(jc, &entry->attr_count)) {
        jc->status = UNXPTD_EOF;
        return 0;
    }

    if (!checkMethodAccessFlags(jc, entry->access_flags))
        return 0;

    if (entry->name_index == 0 || entry->name_index >= jc->constantPoolCount || !methodnameIdxIsValid(jc, entry->name_index)) {
        jc->status = INV_NAME_IDX;
        return 0;
    }

    cp_info* cpi = jc->constantPool + entry->descriptor_index - 1;

    if (entry->descriptor_index == 0 || entry->descriptor_index >= jc->constantPoolCount || cpi->tag != CONST_Utf8 || cpi->Utf8.length != readMethodDesc(cpi->Utf8.bytes, cpi->Utf8.length, 1)) {
        jc->status = INV_FIELD_DESC_IDX;
        return 0;
    }

    if (entry->attr_count > 0) {
        entry->attributes = (attribute_info*)malloc(sizeof(attribute_info) * entry->attr_count);
        if (!entry->attributes) {
            jc->status = MEM_ALLOC_FAILED;
            return 0;
        }

        uint16_t i;

        jc->attributeEntriesRead = 0;

        for (i = 0; i < entry->attr_count; i++) {
            if (!readAttribute(jc, entry->attributes + i)) {
                entry->attr_count = i + 1;
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
        for (i = 0; i < entry->attr_count; i++){
            freeAttributeInfo(entry->attributes + i);
        }

        free(entry->attributes);

        entry->attr_count = 0;
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
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);

            printf("  Method #%u:\n\n", u16);
            printf("    Name:        #%u <%s>\n", mi->name_index, buffer);
            cpi = jc->constantPool + mi->descriptor_index - 1;
            UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);
            printf("    Descriptor:  #%u <%s>\n", mi->descriptor_index, buffer);
            decodeAccessFlags(mi->access_flags, buffer, sizeof(buffer), ACCT_METHOD);
            printf("    Access flags:      0x%.4X [%s]\n", mi->access_flags, buffer);
            printf("    Attribute Count:   %u\n", mi->attr_count);

            if (mi->attr_count > 0) {
                for (att_index = 0; att_index < mi->attr_count; att_index++) {
                    atti = mi->attributes + att_index;
                    cpi = jc->constantPool + atti->name_index - 1;
                    UTF8ToASCII((uint8_t*)buffer, sizeof(buffer), cpi->Utf8.bytes, cpi->Utf8.length);

                    printf("    Method Attribute #%u - %s:\n", att_index + 1, buffer);
                    printAttribute(jc, atti, 2);
                }
            }
        }
    }
}

method_info* getMethodMatch(JavaClass* jc, const uint8_t* name, int32_t name_len, const uint8_t* descriptor,
                               int32_t desc_len, uint16_t flag_mask) {
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
        if (!cmp_UTF8_Ascii(cpi->Utf8.bytes, cpi->Utf8.length, descriptor, desc_len))
            continue;

        return method;
    }
    return NULL;
}
