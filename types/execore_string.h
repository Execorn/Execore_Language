//
// Created by Legion on 08.12.2021.
//

#ifndef EXECORE_STR_H
#define EXECORE_STR_H

#include "target.h"
#include "src/number.h"

typedef struct {
	TARGET_INFO_D;
	char* string_ptr;
} TargetStr;

typedef struct {
    TYPE_FUNC;
    
	Target* (*len)(TargetStr *obj);
	TargetChar *(*content)(TargetStr *str, int_t index);
	TargetStr *(*slice)(TargetStr *obj, int_t start, int_t end);
	Target* (*cat)(Target* left_operand, Target* right_operand);
	Target* (*clone)(Target* left_operand, Target* right_operand);
	Target* (*equal)(Target* left_operand, Target* right_operand);
	Target* (*not_equal)(Target* left_operand, Target* right_operand);
} StrType;

extern StrType StrT;

#endif
