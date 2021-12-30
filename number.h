//
// Created by Legion on 04.12.2021.
//

#ifndef EXECORE_NUMBER_H
#define EXECORE_NUMBER_H

#include "src/types/target.h"

typedef struct {
	TARGET_INFO_D;
	char_t char_value;
} TargetChar;

typedef struct {
	TARGET_INFO_D;
	int_t int_value;
} TargetInt;

typedef struct {
	TARGET_INFO_D;
	float_t float_value;
} TargetFloat;

typedef struct {
    TYPE_FUNC;
} Char_T;

extern Char_T charT;

typedef struct {
    TYPE_FUNC;
} Int_T;

extern Int_T intT;

typedef struct {
    TYPE_FUNC;
} Float_T;

extern Float_T floatT;

typedef struct {
    TYPE_FUNC;

	Target* (*add               )(Target* left_operand, Target* right_operand);
	Target* (*sub               )(Target* left_operand, Target* right_operand);
	Target* (*mul               )(Target* left_operand, Target* right_operand);
	Target* (*div               )(Target* left_operand, Target* right_operand);
	Target* (*mod               )(Target* left_operand, Target* right_operand);
	Target* (*invert            )(Target* left_operand);
	Target* (*equal             )(Target* left_operand, Target* right_operand);
	Target* (*not_equal         )(Target* left_operand, Target* right_operand);
	Target* (*less_t            )(Target* left_operand, Target* right_operand);
	Target* (*less_equal_t      )(Target* left_operand, Target* right_operand);
	Target* (*greater_t         )(Target* left_operand, Target* right_operand);
	Target* (*greater_equal_t   )(Target* left_operand, Target* right_operand);
	Target* (*or                )(Target* left_operand, Target* right_operand);
	Target* (*and               )(Target* left_operand, Target* right_operand);
	Target* (*negate            )(Target* left_operand);
} Num_T;

extern Num_T numT;

#endif
