#ifndef CONSTANTPOOL_H
#define CONSTANTPOOL_H

typedef struct cp_info cp_info;

#include <stdint.h>
#include "javaclass.h"

struct cp_info {

    uint8_t tag;

    union {

        struct {
            uint16_t name_index;
        } Class;

        struct {
            uint16_t string_index;
        } String;

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

enum ConstantPoolTag {
    CONSTANT_Class = 7,
    CONSTANT_Fieldref = 9,
    CONSTANT_Methodref = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_String = 8,
    CONSTANT_Integer = 3,
    CONSTANT_Float = 4,
    CONSTANT_Long = 5,
    CONSTANT_Double = 6,
    CONSTANT_NameAndType = 12,
    CONSTANT_Utf8 = 1,
    // Compatibility with Java 8
    CONSTANT_MethodHandle = 15,
    CONSTANT_MethodType = 16,
    CONSTANT_InvokeDynamic = 18
};

const char* decodeTag(uint8_t tag);
char readConstantPoolEntry(JavaClass* jc, cp_info* entry);
void printConstantPool(JavaClass* jc);

#endif // CONSTANTPOOL_H
