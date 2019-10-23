#ifndef FIELDS_H
#define FIELDS_H

typedef struct field_info field_info;

#include "javaClass.h"

struct field_info {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    uint16_t attributes_count;
    attribute_info* attributes;

    uint16_t offset;
};

char readField(JavaClass* jc, field_info* entry);
void freeFieldAttr(field_info* entry);
void printAllFields(JavaClass* jc);

field_info* getFieldMatch(JavaClass* jc, const uint8_t* name, int32_t name_len, const uint8_t* descriptor,
                             int32_t desc_len, uint16_t flag_mask);

#endif
