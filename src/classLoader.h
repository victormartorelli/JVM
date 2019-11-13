#ifndef CLASSLOADER_H
#define CLASSLOADER_H

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
    JavaClass *jc;
    Stack *stack_list;
    Frame *next;
};

struct {
    LoadedClass *class_list;
    Frame *frame_list;

    char classPath[256];
}

#endif
