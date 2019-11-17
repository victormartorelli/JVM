#ifndef CLASSLOADER_H
#define CLASSLOADER_H

#include <stdint.h>

typedef struct Stack {
    int32_t value;
    int type;
    Stack *next;
};

typedef struct LoadedClass {
    JavaClass *jc;
    LoadedClass *next;
} LoadedClass;

typedef struct Frame {
    uint8_t returnc;
    uint32_t PC;
    uint32_t code_length;
    uint8_t* code;
    int32_t* local_vars;

    JavaClass *jc;
    Stack *stack_list;
    Frame *next;
};

struct ClassLoader {
    LoadedClass *class_list;
    Frame *frame_list;

    char classPath[256];
};

#endif
