
#include <assert.h>
#include <stdlib.h>

#include "stack.h"
#include "src/utils/macro_utils.h"
#include "stdio.h"

static bool IsStackFull(Stack* stack) {
    REPORT(stack);

	return stack->top == stack->capacity - 1;
}

bool IsEmptyStack(Stack* stack) {
    REPORT(stack);

	return stack->top == -1;
}

void StackPush(Stack* stack, void* item) {
    REPORT(stack);

	if (IsStackFull(stack)) {
		stack->capacity += INCREASE_CONST;
        stack->array = realloc(stack->array, stack->capacity * sizeof(void*));
	}
    stack->array[++stack->top] = item;
}

void* StackPop(Stack* stack) {
    REPORT(stack);

	void* p   = NULL;
    void* mem = NULL;

	if (IsEmptyStack(stack)) {
        return NULL;
    }

	p = stack->array[stack->top--];

	if (stack->top + 1 - stack->capacity >= DECREASE_CONST)
		if ((mem = realloc(stack->array, stack->top + 1)) != NULL) {
            stack->array = mem;
        }

	return p;
}

void* StackPeek(Stack* stack) {
    REPORT(stack);

	if (IsEmptyStack(stack))
		return NULL;

	return stack->array[stack->top];
}

Stack* StackConstructor(long capacity) {
	Stack* stack = NULL;

	if ((stack = (Stack*) calloc(1, sizeof(Stack))) != NULL) {
		stack->capacity = capacity > 0 ? capacity : 0;
		stack->top = -1;

		if ((stack->array = calloc(stack->capacity, sizeof(void*))) == NULL) {
			assert("[!] Calloc error [!]" && 0);
		}
	}
	return stack;
}

void StackDestructor(Stack* stack) {
    REPORT(stack);

	if (stack->array)
		free(stack->array);

	free(stack);
}
