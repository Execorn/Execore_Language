//
// Created by Legion on 08.12.2021.
//

#ifndef EXECORE_FUNCTION_H
#define EXECORE_FUNCTION_H

#include "array.h"
#include "stack.h"

bool IsBuiltinFunc   (const char* func_name);
size_t GetBuiltinArgc(const char* func_name);
void CompileBuiltin  (const char* func_name, Array* args, Stack* stack);

#endif
