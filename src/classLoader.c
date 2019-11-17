#include "classLoader.h"

uint8_t pushToStack(Stack** stack, int32_t operand, int type){
    Stack* new_operand = (Stack*)malloc(sizeof(Stack));

    //https://stackoverflow.com/questions/5607455/checking-that-malloc-succeeded-in-c

    if(new_operand){
        new_operand->operand = operand;
        new_operand->type = type;
        new_operand->next = *stack;
        *stack = new_operand;
    }

    return new_operand != NULL;
}

void initClassLoader(ClassLoader* cl)
{
    return;
}
void freeClassLoader(ClassLoader* cl)
{
    return;
}
