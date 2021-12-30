//
// Created by Legion on 01.12.2021.
//

#include <string.h>

#include "src/types/node_list.h"
#include "handle_err.h"
#include "function.h"
#include "src/utils/macro_utils.h"


/*
 * type(expr)
 */
static void builtin_type(Array* arguments, Stack* stack) {
    REPORT(arguments);
    REPORT(stack);

	Target* target = arguments->data[0];

	Target* result = IS_LIST(target) ? TypeTarget(LIST_TARGET(target)) : TypeTarget(target);

    DecreaseUsages(target);

    StackPush(stack, result);
}


/* 
 * chr(integer expression)
 */
static void builtin_chr(Array* arguments, Stack* stack) {
    REPORT(arguments);
    REPORT(stack);

	char buffer[MAX_NUM_LENGTH];

	Target* obj = arguments->data[0];

	snprintf(buffer, sizeof(buffer), "%c", TargetToChar(obj));
	Target* result = MakeTarget(STR_T, buffer);

    DecreaseUsages(obj);

    StackPush(stack, result);
}


/*
 * ord(str expr)
 */
static void builtin_ord(Array* arguments, Stack* stack) {
    REPORT(arguments);
    REPORT(stack);

	Target* current_object = arguments->data[0];

	if (TYPE(current_object) != STR_T) {
        RaiseError(TYPE_ERROR, "expected string but found %stack", TARGET_NAME(current_object));
    }

	Target* result = MakeTarget(INT_T, (int_t) TargetToChar(current_object));

    DecreaseUsages(current_object);

    StackPush(stack, result);
}

static struct {
	char* func_name;
	size_t argc;
	void (*func_ptr)(Array*, Stack*);
} builtins[] = {
	{"chr",  1, builtin_chr},
	{"ord",  1, builtin_ord},
	{"type", 1, builtin_type}
};

static int FindBuiltin(const char* func_name) {
    REPORT(func_name);

    int left   = 0, right          = (sizeof builtins / sizeof builtins[0]) - 1;
    int middle = 0, compare_result = 0;

	while (left <= right) {
        middle = (left + right) / 2;
        compare_result = strcmp(&func_name[0], builtins[middle].func_name);
		if (compare_result < 0) {
            right = middle - 1;
        }
		if (compare_result > 0) {
            left = middle + 1;
        }
		if (!compare_result) {
            break;
        }
	}
	if (!compare_result) return middle;

	return -1;
}

void CompileBuiltin(const char* func_name, Array* args, Stack* stack) {
    REPORT(func_name);
	if (!IsBuiltinFunc(func_name)) {
        MAKE_ERR("IsBuiltinFunc(%s) = false. Error occurred!", NAME(func_name));
    }

	int index = FindBuiltin(func_name);
	if (index >= 0) {
        builtins[index].func_ptr(args, stack);
    }
}

bool IsBuiltinFunc(const char* func_name) {
    REPORT(func_name);

	return FindBuiltin(func_name) != -1;
}

size_t GetBuiltinArgc(const char* func_name) {
    REPORT(func_name);

    //TODO: change to FindBuiltin
    if (!IsBuiltinFunc(func_name)) {
        MAKE_ERR("IsBuiltinFunc(%s) = false. Error occurred!", NAME(func_name));
    }

	return builtins[FindBuiltin(func_name)].argc;
}
