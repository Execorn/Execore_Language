//
// Created by Legion on 08.12.2021.
//

#include <stdlib.h>
#include <string.h>

#include "src/handle_err.h"
#include "execore_string.h"
#include "src/utils/macro_utils.h"


static const TargetStr EMPTY_TARGET = (const TargetStr) {0};

static TargetStr* ConstructString() {
	TargetStr* target = NULL;

	if ((target = calloc(1, sizeof(TargetStr))) != NULL) {
		target->target_type     = (TargetType *)&StrT;
		target->type        = STR_T;
		target->usages    = 0;

		if ((target->string_ptr = strdup("")) == NULL) {
            FreeTarget((Target*) target);
			target = NULL;
		}
	}
	return target;
}

static void FreeString(TargetStr* target) {
    REPORT(target);

	free(target->string_ptr);
	*target = EMPTY_TARGET;

	free(target);
}


static void DumpString(FILE* dump_file, TargetStr* target) {
    REPORT(dump_file);
    REPORT(target);
    
	fprintf(dump_file, "%s", target->string_ptr);
}


static void InitString(TargetStr* target, const char* cur_str) {
    REPORT(target);
    REPORT(cur_str);

	if (target->string_ptr) {
        free(target->string_ptr);
    }

	if ((target->string_ptr = strdup(cur_str)) == NULL) {
        RaiseError(MEM_ERROR);
    }
}


static void StringAdv_init(TargetStr* target, va_list va_args) {
    InitString(target, va_arg(va_args, char*));
}

static Target* GetStringMethod(TargetStr* target, char* name, Array* arguments) {
    REPORT(target);
    REPORT(name);
    REPORT(arguments);

	NAEBAL_GCC(arguments);

	Target* result = NULL;
	if (!strcmp("len", name)) {
		if (arguments->current_index) {
            RaiseError(SYNTAX_ERROR, "give %s %d args pls...", name, 2);
			result = ConstructTarget(NAN_T);
		} else
			result = StrT.len(target);
	} else {
        RaiseError(SYNTAX_ERROR, "where the fuck did u find method %s in %s?", TARGET_NAME(target), name);
		result = ConstructTarget(NAN_T);
	}

	return result;
}

static Target* GetStrLen(TargetStr* target) {
    REPORT(target);

	Target* target_len = NULL;

	if ((target_len = MakeTarget(INT_T, strlen(target->string_ptr))) == NULL) {
        target_len = ConstructTarget(NAN_T); // as always
    }

	return target_len;
}

static Target* StrCar(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	char* new_str       = NULL;
	size_t bytes        =    0;
	Target* target      = NULL;
	TargetStr* str1     = NULL;
    TargetStr* str2     = NULL;
    TargetStr* content  = NULL;

    str1 = TYPE(left_operand)  == STR_T ? (TargetStr*)left_operand  : 
            (content = (TargetStr*)TargetContentToStr(left_operand));
    str2 = TYPE(right_operand) == STR_T ? (TargetStr*)right_operand : 
            (content = (TargetStr*)TargetContentToStr(right_operand));

	bytes = strlen(str1->string_ptr) + strlen(str2->string_ptr) + 1;
    
	if ((new_str = calloc(bytes, sizeof(char))) == NULL) {
        RaiseError(MEM_ERROR);
		return ConstructTarget(NAN_T);
	}

	strcpy(new_str, str1->string_ptr);
	strcat(new_str, str2->string_ptr);

	if ((target = MakeTarget(STR_T, new_str)) == NULL) {
        target = ConstructTarget(NAN_T);
    }
	free(new_str);

	if (content) FreeTarget((Target*) content);

	return target;
}

static Target* StrClone(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	char* str       = NULL;
	size_t bytes    =    0;
	Target* target  = NULL;

	TargetStr* str1 = TYPE(left_operand) == STR_T ? (TargetStr*)left_operand : (TargetStr*)right_operand;
	Target* new_str = TYPE(left_operand) == STR_T ? right_operand : left_operand;

	int_t times = TargetToInt(new_str);

	if (times < 0) times = !times;

	bytes = strlen(str1->string_ptr) * times + 1;

	if ((str = calloc(bytes, sizeof(char))) == NULL) {
        RaiseError(MEM_ERROR);
		return ConstructTarget(NAN_T);
	}

	while (times--) strcat(str, str1->string_ptr);

	if ((target = MakeTarget(STR_T, str)) == NULL)
		target = ConstructTarget(NAN_T);

	free(str);

	return target;
}

static Target* StrEqual(TargetStr* left_operand, TargetStr* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    Target* result = NULL;

	result = MakeTarget(INT_T, (int_t) (!strcmp(left_operand->string_ptr, right_operand->string_ptr)));
	if (!result) result = ConstructTarget(NAN_T);

	return result;
}

static Target* StrNotEqual(TargetStr* left_operand, TargetStr* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    Target* result = NULL;

	result = MakeTarget(INT_T, (int_t) (strcmp(left_operand->string_ptr, right_operand->string_ptr) != 0));
	if (!result) result = ConstructTarget(NAN_T);

	return result;
}

static TargetChar* GetStrContent(TargetStr* target, int_t index) {
    REPORT(target);

	TargetChar* symbol = 0;
	int_t cur_len      = 0;

    cur_len = (int_t) strlen(target->string_ptr);

	if (index < 0) index += cur_len;

	if (index < 0 || index >= cur_len) {
        RaiseError(INDEX_ERROR);
		return (TargetChar*) ConstructTarget(NAN_T);
	}

	if ((symbol = (TargetChar*) MakeTarget(CHAR_T, *(target->string_ptr + index))) == NULL) {
        symbol = (TargetChar*) ConstructTarget(NAN_T);
    }

	return symbol;
}

static TargetStr* GetStrSlice(TargetStr* target, int_t start, int_t end) {
    REPORT(target);

	TargetStr* slice = NULL;
	char* str        = NULL;

	int_t cur_len = (int_t) strlen(target->string_ptr);

	if (start < 0) start += cur_len;
	if (end < 0) end += cur_len;
	if (start < 0) start = 0;
	if (end >= cur_len) end = cur_len;

	if ((str = strndup(target->string_ptr + start, end - start)) == NULL) {
        return (TargetStr*) ConstructTarget(NAN_T);
    }

	if ((slice = (TargetStr*) MakeTarget(STR_T, str)) == NULL) {
        slice = (TargetStr*) ConstructTarget(NAN_T);
    }

	free(str);
	return slice;
}

StrType StrT = {
	.name       = "str",
	.construct  = (Target* (*)()) ConstructString,
	.free       = (void (*)(Target*)) FreeString,
	.dump       = (void (*)(FILE *, Target* )) DumpString,
	.init       = InitString,
	.adv_init   = (void (*)(Target* , va_list)) StringAdv_init,
	.method     = (Target* (*)(Target* , char* , Array *)) GetStringMethod,

	.len        = GetStrLen,
	.content    = GetStrContent,
	.slice      = GetStrSlice,
	.cat        = StrCar,
	.clone      = StrClone,
	.equal      = (Target* (*)(Target* , Target* )) StrEqual,
	.not_equal  = (Target* (*)(Target* , Target* )) StrNotEqual,
};
