#ifndef CONSTANTPOOL_H
#define CONSTANTPOOL_H

typedef struct cp_info cp_info;

#include "javaClass.h"

struct cp_info {

    uint8_t tag;

    union {
        struct {
            uint16_t string_index;
        } String;

        struct {
            uint16_t name_index;
        } Class;

        struct {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } Fieldref;

        struct {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } Methodref;

        struct {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } InterfaceMethodref;

        struct {
            uint16_t name_index;
            uint16_t descriptor_index;
        } NameAndType;

        struct {
            uint32_t value;
        } Integer;

        struct {
            uint32_t bytes;
        } Float;

        struct {
            uint32_t high;
            uint32_t low;
        } Long;

        struct {
            uint32_t high;
            uint32_t low;
        } Double;

        struct {
            uint16_t length;
            uint8_t* bytes;
        } Utf8;
    };
};

enum CPTag {
    CONST_Class = 7,
    CONST_Fieldref = 9,
    CONST_Methodref = 10,
    CONST_InterfaceMethodref = 11,
    CONST_String = 8,
    CONST_Integer = 3,
    CONST_Float = 4,
    CONST_Long = 5,
    CONST_Double = 6,
    CONST_NameAndType = 12,
    CONST_Utf8 = 1,
    
    CONST_MethodHandle = 15,
    CONST_MethodType = 16,
    CONST_InvokeDynamic = 18
};

const char* decodeTag(uint8_t tag);
char readCPEntry(JavaClass* jc, cp_info* entry);
void printCP(JavaClass* jc);

#endif
