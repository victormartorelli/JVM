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

// Nao testado -> rofl
uint8_t popFromStack(Stack** stack, int32_t* out_operand, int* out_type){
    Stack* pop_operand = (Stack*)malloc(sizeof(Stack));

    if(pop_operand){
        if(out_operand){
            *out_operand = pop_operand->operand;
        }
        if(out_type){
            *out_type = pop_operand->type;
        }
        *stack = pop_operand->next;
        free(pop_operand);
    }
    return pop_operand != NULL;
}
// Nao testado -> rofl
void freeStack(Stack** stack){
    Stack* operand = *stack;
    Stack* aux;

    while(stack){
        aux = stack;
        stack = stack->next;
        free(aux);
    }
    *stack = NULL;
}



void initClassLoader(ClassLoader* cl)
{
    return;
}
void freeClassLoader(ClassLoader* cl)
{
    return;
}
