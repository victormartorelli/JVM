#include "instructions.h"
#include "utf8.h"
#include "jvm.h"
#include <math.h>

#define NEXT_BYTE (*(frame->code + frame->pc++))
#define HIWORD(x) ((int32_t)(x >> 32))
#define LOWORD(x) ((int32_t)(x & 0xFFFFFFFFll))

uint8_t instfunc_nop(JavaVirtualMachine* jvm, Frame* frame){
    return 1;
}

uint8_t instfunc_aconst_null(JavaVirtualMachine* jvm, Frame* frame){
    if (!pushOperand(&frame->operands, 0, OP_REFERENCE))
    {
        jvm->status = JVM_STATUS_OUT_OF_MEMORY;
        return 0;
    }

    return 1;
}