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

void freeFrame(Frame* frame){
    if(frame->local_vars){
        free(frame->local_vars);
    }
    if(frame->stack_list){
        freeStack(&frame->stack_list);
    }
    free(frame);
}
// Nao sei se ta 100% correto essa do pushFrame
// O que eu n sei se ta certo é o fato do in_obj acessar o frame. 

uint8_t pushFrame(Frame** f, Frame* frame){
    Frame* in_obj = (Frame*)malloc(sizeof(Frame));

    if(in_obj){
        in_obj->frame = frame;
        in_obj->next = *f;
        *f = in_obj;
    }
    return in_obj != NULL;
}
// Nao sei se ta 100% correto essa do popFrame
// O que eu n sei se ta certo é o fato do out_obj acessar o frame. 
uint8_t pop_frame(Frame** f, Frame* output_frame){
    Frame* out_obj = *f;
    if(out_obj){
        if(output_frame){
            *output_frame = *out_obj->frame;
        }
        *f = out_obj->next;
        free(out_obj);
    }
    return out_obj != NULL;
}




void initClassLoader(ClassLoader* cl)
{
    return;
}
void freeClassLoader(ClassLoader* cl)
{
    return;
}
