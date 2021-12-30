//
// Created by Legion on 02.12.2021.
//

#ifndef EXECORE_STACK_H
#define EXECORE_STACK_H

#include <stdbool.h>

#define INCREASE_CONST 10
#define DECREASE_CONST 100

#include "src/utils/struct_info.h"
typedef struct stack {
	STACK_INFO;
} Stack;

extern Stack* StackConstructor(long capacity);
extern void StackDestructor(Stack* stack);

extern void  StackPush(Stack* stack, void* item);
extern void* StackPop(Stack* stack);
extern void* StackPeek(Stack* stack);
extern bool  IsEmptyStack(Stack* stack);

#endif
