//
// Created by Legion on 04.12.2021.
//

#include <stdlib.h>

#include "number.h"
#include "handle_err.h"
#include "src/utils/macro_utils.h"

const Target FREE_TARGET = (const Target) {0};


static Target* ConstructCharTarget() {
	TargetChar* target = NULL;

	if ((target = calloc(1, sizeof(TargetChar))) != NULL) {
		target->target_type= (TargetType*) &charT;
		target->type       = CHAR_T;
		target->usages     = 0;
		target->char_value = 0;
	}
	return (Target*) target;
}


static Target* ConstructIntTarget() {
	TargetInt* target = NULL;

	if ((target = calloc(1, sizeof(TargetInt))) != NULL) {
		target->target_type   = (TargetType*) &intT;
		target->type      = INT_T;
		target->usages    = 0;
		target->int_value = 0;
	}
	return (Target*) target;
}


static Target* ConstructFloatTarget() {
	TargetFloat* target = NULL;

	if ((target = calloc(1, sizeof(TargetFloat))) != NULL) {
		target->target_type     = (TargetType*) &floatT;
		target->type        = FLOAT_T;
		target->usages    = 0;
		target->float_value = 0;
	}
	return (Target*) target;
}


static void FreeNum(Target* target) {
    REPORT(target);
    
	*target = FREE_TARGET; // POISONING

	free(target);
}


/* A double argument representing a floating-point number is converted in style f or e
 * (or in style F or E in the case of a G conversion specifier),
 * depending on the value converted and the precision. Let P equal the precision if nonzero,
 * 6 if the precision is omitted, or 1 if the precision is zero.
 * Then, if a conversion with style E would have an exponent of X:
 *      if P > X ≥ −4, the conversion is with style f (or F) and precision P − (X + 1).
 *      otherwise, the conversion is with style e (or E) and precision P − 1.
 * Finally, unless the # flag is used,
 * any trailing zeros are removed from the fractional portion of the result and the decimal-point character is removed
 * if there is no fractional portion remaining.
 * A double argument representing an infinity or NaN is converted in the style of an f or F conversion specifier.
 */

static void NumDump(FILE* dump_file, Target* target) {
    REPORT(target);
    
	switch (TYPE(target)) {
		case CHAR_T:
			fprintf(dump_file, "%c", TargetToChar(target));
			break;

		case INT_T:
			fprintf(dump_file, "%ld", TargetToInt(target));
			break;

		case FLOAT_T:
			fprintf(dump_file, "%.*G", 15, TargetToFloat(target));
			break; /* округление до 15 знаков после запятой, нули не добавляются */

		default:
            RaiseError(LANG_ERROR);
            break;
	}
}


static void InitChar(TargetChar* target, char_t symbol) {
    REPORT(target);
    
	target->char_value = symbol;
}


static void InitInt(TargetInt* target, int_t number) {
    REPORT(target);
    
	target->int_value = number;
}


static void InitFloat(TargetFloat* target, float_t f_number) {
    REPORT(target);
    
	target->float_value = f_number;
}


static void NumAdv_Init(Target* target, va_list va_args) {
    REPORT(target);
    
	switch (TYPE(target)) {
		case CHAR_T:
            InitChar((TargetChar*) target, va_arg(va_args, int));
			break;

		case INT_T:
            InitInt((TargetInt*) target, va_arg(va_args, int_t));
			break;

		case FLOAT_T:
            InitFloat((TargetFloat*) target, va_arg(va_args, float_t));
			break;

		default:
            RaiseError(LANG_ERROR);
            break;
	}
}


static Target* NumMethodError(Target* target, char* name, Array* args) {
    REPORT(target);
    REPORT(name);
    REPORT(args);

	NAEBAL_GCC(args);

    RaiseError(SYNTAX_ERROR, "lol wtf? %s has no method %s, wdym dude?", TARGET_NAME(target), name);

	return ConstructTarget(NAN_T);
}

static target_t DetermineResultType(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	if (TYPE(left_operand) == FLOAT_T || TYPE(right_operand) == FLOAT_T) {
        return FLOAT_T;
    } else if (TYPE(left_operand) == INT_T || TYPE(right_operand) == INT_T) {
        return INT_T;
    } else return CHAR_T;
}

#define NUM_FUNC_GENERATOR(name, op) \
static Target* Num##name(Target* left_operand, Target* right_operand) {\
    REPORT(left_operand);\
    REPORT(right_operand);\
    \
    Target* result = NULL;\
\
    switch (DetermineResultType(left_operand, right_operand)) {\
    case CHAR_T:\
        result = MakeTarget(CHAR_T, TargetToChar(left_operand) op TargetToChar(right_operand));\
        break;\
\
    case INT_T:\
        result = MakeTarget(INT_T, TargetToInt(left_operand) op TargetToInt(right_operand));\
        break;\
    \
    case FLOAT_T:\
        result = MakeTarget(FLOAT_T, TargetToFloat(left_operand) op TargetToFloat(right_operand));\
        break;\
    \
    default:\
        RaiseError(LANG_ERROR); /* если мы тут оказались, то пиздец */\
        break;\
    }\
    \
    if (!result) result = ConstructTarget(NAN_T);\
\
    return result; /* NULL should NEVER be returned */ \
}

NUM_FUNC_GENERATOR(Add, +)

NUM_FUNC_GENERATOR(Sub, -)

NUM_FUNC_GENERATOR(Mul, *)

NUM_FUNC_GENERATOR(Div, /)

#undef NUM_FUNC_GENERATOR

//TODO: make a way to put Mod, Inv, Eql etc. in NUM_FUNC_GENERATOR()
static Target* NumMod(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	Target* result = NULL;

	if (TargetToFloat(right_operand) == 0) {
        RaiseError(ZERO_DIVISION_ERROR);
    } else {
        switch (DetermineResultType(left_operand, right_operand)) {
            case CHAR_T:
                result = MakeTarget(CHAR_T, TargetToChar(left_operand) % TargetToChar(right_operand));
                break;

            case INT_T:
                result = MakeTarget(INT_T, TargetToInt(left_operand) % TargetToInt(right_operand));
                break;

            case FLOAT_T:
                RaiseError(OPERATOR_WITH_WRONG_TYPE, "STOP USING %% this with float, WHAT do u want to get?");
                break;

            default:
                RaiseError(LANG_ERROR);
                break;
        }
    }

	if (!result) result = ConstructTarget(NAN_T);

	return result;
}


static Target* NumInvert(Target* value) {
    REPORT(value);

	Target* right_operand = NULL;
    Target* result        = NULL;

	switch (TYPE(value)) {
		case CHAR_T:
			right_operand = MakeTarget(CHAR_T, (char_t) 0);
			break;

		case INT_T:
			right_operand = MakeTarget(INT_T, (int_t) 0);
			break;

		case FLOAT_T:
			right_operand = MakeTarget(FLOAT_T, (float_t) 0);
			break;

		default:
            RaiseError(LANG_ERROR);
            break;
	}

	if (right_operand) {
        result = SubTarget(right_operand, value);
        DecreaseUsages(right_operand);
	} else result = ConstructTarget(NAN_T);

	return result;
}

#define NUM_RELATE_GENERATOR(name, op) \
static Target* Num##name(Target* left_operand, Target* right_operand) { \
    REPORT(left_operand);                                   \
    REPORT(right_operand);             \
    \
    Target* result = NULL;\
    \
    if (TYPE(left_operand) == FLOAT_T || TYPE(right_operand) == FLOAT_T) { \
        result = MakeTarget(INT_T, (int_t) (TargetToFloat(left_operand) op TargetToFloat(right_operand))); \
    } else if (TYPE(left_operand) == INT_T || TYPE(right_operand) op INT_T) {\
        result = MakeTarget(INT_T, (int_t) (TargetToInt(left_operand) op TargetToInt(right_operand)));      \
    } else result = MakeTarget(INT_T, (int_t) (TargetToChar(left_operand) op TargetToChar(right_operand)));\
    \
    if (!result) result = ConstructTarget(NAN_T);\
    \
    return result;\
}


NUM_RELATE_GENERATOR(Equal, ==)

NUM_RELATE_GENERATOR(NotEqual, !=)

NUM_RELATE_GENERATOR(Less, <)

NUM_RELATE_GENERATOR(LessEqual, <=)

NUM_RELATE_GENERATOR(Greater, >)

NUM_RELATE_GENERATOR(GreaterEqual, >=)

#undef NUM_RELATE_GENERATOR


static Target* NumLogOr(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    Target* result = NULL;

	result = MakeTarget(INT_T, (int_t) (TargetToBool(left_operand) || TargetToBool(right_operand)));

	if (!result) result = ConstructTarget(NAN_T);

	return result;
}


static Target* NumLogAnd(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    Target* result = NULL;

	result = MakeTarget(INT_T, (int_t) (TargetToBool(left_operand) && TargetToBool(right_operand)));

	if (!result) result = ConstructTarget(NAN_T);

	return result;
}


static Target* NumNegate(Target* left_operand) {
    REPORT(left_operand);

	Target* result = NULL;

	result = MakeTarget(INT_T, (int_t) !TargetToBool(left_operand));

	if (!result) result = ConstructTarget(NAN_T);

	return result;
}


Char_T charT = {
	.name           = "char",
	.construct      = ConstructCharTarget,
	.free           = FreeNum,
	.dump           = NumDump,
	.init           = (void (*)())InitChar,
	.adv_init       = NumAdv_Init,
	.method         = NumMethodError,
};

Int_T intT = {
	.name           = "int",
	.construct      = ConstructIntTarget,
	.free           = FreeNum,
	.dump           = NumDump,
	.init           = InitInt,
	.adv_init       = NumAdv_Init,
	.method         = NumMethodError,
};

Float_T floatT = {
	.name           = "float",
	.construct      = ConstructFloatTarget,
	.free           = FreeNum,
	.dump           = NumDump,
	.init           = InitFloat,
	.adv_init       = NumAdv_Init,
	.method         = NumMethodError,
};

Num_T numT = {
	.name           = "number",
	.construct      = ConstructIntTarget,
	.free           = FreeNum,
	.dump           = NumDump,
	.init           = InitInt,
	.adv_init       = NumAdv_Init,
	.method         = NumMethodError,

	.add            = NumAdd,
	.sub            = NumSub,
	.mul            = NumMul,
	.div            = NumDiv,
	.mod            = NumMod,
	.invert         = NumInvert,
	.equal          = NumEqual,
	.not_equal      = NumNotEqual,
	.less_t         = NumLess,
	.less_equal_t   = NumLessEqual,
	.greater_t      = NumGreater,
	.greater_equal_t= NumGreaterEqual,
	.or             = NumLogOr,
	.and            = NumLogAnd,
	.negate         = NumNegate,
};
