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

uint8_t pushFrame(JavaClass* jc, method_info* method, Frame* frame){
    Frame* frame = (Frame*)malloc(sizeof(Frame));

    if(frame){
        attribute_info* code_attribute = getAttributeByType(method->attributes, method->attr_count, ATTR_Code);
        att_Code_info* code;
        if(code_attribute){
            code = (att_Code_info*)code_attribute->info;
            frame->code = code->code;
            frame->code_length = code->code_length;
            if(code->max_locals > 0){
                frame->local_vars = (int32_t*)malloc(code->max_locals*sizeof(int32_t));
            }else{
                frame->local_vars = NULL;
            }
        }else{
            frame->code = NULL;
            frame->code_length = 0;
            frame->local_vars = NULL;
        }
        frame->operands = NULL;
        frame->jc = jc;
        frame->pc = 0;
    }
    return frame
}

uint8_t pop_frame(JavaClass* jc, method_info* method, Frame* frame){

}

void freeFrame(Frame* frame){
    if(frame->local_vars){
        free(frame->local_vars);
    }
    if(frame->stack_list){
        freeStack(&frame->stack_list);
    }
    free(frame);
}

Frame* newFrame(JavaClass* jc, method_info* method){
    Frame* frame = (Frame*)malloc(sizeof(Frame));

    if(frame){
        attribute_info* code_attribute = getAttributeByType(method->attributes, method->attr_count, ATTR_Code);
        att_Code_info* code;
        if(code_attribute){
            code = (att_Code_info*)code_attribute->info;
            frame->code = code->code;
            frame->code_length = code->code_length;
            if(code->max_locals > 0){
                frame->local_vars = (int32_t*)malloc(code->max_locals*sizeof(int32_t));
            }else{
                frame->local_vars = NULL;
            }
        }else{
            frame->code = NULL;
            frame->code_length = 0;
            frame->local_vars = NULL;
        }
        frame->operands = NULL;
        frame->jc = jc;
        frame->pc = 0;
    }
    return frame;
}



void initClassLoader(ClassLoader* cl)
{
    return;
}
void freeClassLoader(ClassLoader* cl)
{
    return;
}
