#ifndef CLASSLOADER_H
#define CLASSLOADER_H

#include <stdint.h>
#include "javaClass.h"

typedef struct Stack Stack;
typedef struct LoadedClass LoadedClass;
typedef struct Frame Frame;
typedef struct ClassLoader ClassLoader;

typedef struct Stack {
    int32_t operand;
    int type;
    Stack *next;
} Stack;

struct LoadedClass {
    JavaClass *jc;
    LoadedClass *next;
};

struct Frame {
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

//operand stack manipulation
uint8_t pushToStack(Stack**,int32_t,int);
uint8_t popFromStack(Stack**,int32_t*,int*);
void freeStack(Stack**);

//frame manipulation

uint8_t pushFrame(JavaClass* jc, method_info* method, Frame* frame);
uint8_t popFrame(JavaClass* jc, method_info* method, Frame* frame);
void freeFrame(Frame* frame);

//JVM manipulation
void initClassLoader(ClassLoader*);
void freeClassLoader(ClassLoader*);


#endif
