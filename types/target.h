//
// Created by Legion on 03.12.2021.
//

#ifndef EXECORE_TARGET_H
#define EXECORE_TARGET_H

#include <stdarg.h>
#include <stdbool.h>
#include "src/array.h"
#include "src/utils/globals.h"

typedef enum {
    CHAR_T = 1,
    INT_T,
    FLOAT_T,
    STR_T,
    LIST_T,
    LST_NODE_T, 
    NAN_T
} target_t;

#ifdef _LANG_DEBUG_
	#define TARGET_INFO_D \
        int usages;  \
        target_t type;  \
        struct type_target* target_type;  \
        struct target* next;  \
        struct target* prev
#else
	#define TARGET_INFO_D \
        int usages;  \
		target_t type;  \
		struct type_target* target_type
#endif


typedef struct target {
    //TARGET_INFO_D
    int usages;
    target_t type;
    struct type_target* target_type;
    struct target* next;
    struct target* prev;
} Target;


#define TYPE_FUNC \
    char* name;\
    Target* (*construct)();\
    void (*free)(Target* target);\
    void (*dump)(FILE* dump_file, Target* target);\
    void (*init)();\
    void (*adv_init)(Target* target, va_list va_args);\
    Target* (*method)(Target* target, char* name, Array* arguments)

typedef struct type_target {
	TYPE_FUNC;
} TargetType;

#define TYPE(target)		(((Target* )(target))->type)
#define TARGET_TYPE(target)	(((Target* )(target))->target_type)
#define TARGET_NAME(target)	(((Target* )(target))->target_type->name)

#define IS_NUM(target)	(TYPE(target) == CHAR_T || TYPE(target) == INT_T || TYPE(target) == FLOAT_T)
#define IS_STR(target)	(TYPE(target) == STR_T)
#define IS_LIST_T(target)		(TYPE(target) == LIST_T)
#define IS_SEQ(target)	(TYPE(target) == LIST_T || TYPE(target) == STR_T)
#define IS_LIST(target)	(TYPE(target) == LST_NODE_T)


extern Target* ConstructTarget(target_t type);
extern Target* MakeTarget(target_t type, ...);
extern void    FreeTarget(Target* target);
extern Target* GetTargetMethod(Target* target, char* method, Array* arguments);
extern void    DumpTarget(FILE* dump_file, Target* cur_trg);
extern Target* GetTarget(FILE* dump_file, target_t type);


static inline void IncreaseUsages(void* target) {
	((Target*) (target))->usages++;
}


static inline void DecreaseUsages(void* target) {
	if (--((Target*) target)->usages <= 0) {
        FreeTarget((Target *) target);
    }
}


extern void    AssignTarget(Target* a, Target* b);
extern Target* CopyTarget(Target* cur_trg);

extern Target* AddTarget(Target* l_opnd, Target* r_opnd);
extern Target* SubTarget(Target* l_opnd, Target* r_opnd);
extern Target* MulTarget(Target* l_opnd, Target* r_opnd);
extern Target* DivTarget(Target* l_opnd, Target* r_opnd);
extern Target* ModTarget(Target* left_operand, Target* right_operand);
extern Target* EqualTarget(Target* left_operand, Target* right_operand);

extern Target* NotEqualTarget(Target* left_operand, Target* r_opnd);
extern Target* LessTarget(Target* l_opnd, Target* r_opnd);
extern Target* LessEqualTarget(Target* l_opnd, Target* r_opnd);
extern Target* GreaterTarget(Target* l_opnd, Target* r_opnd);
extern Target* GreaterEqualTarget(Target* l_opnd, Target* r_opnd);
extern Target* LogOrTarget(Target* l_opnd, Target* r_opnd);
extern Target* LogAndTarget(Target* l_opnd, Target* r_opnd);

extern Target* LogInTarget(Target* l_opnd, Target* r_opnd);

extern Target* NegateTarget(Target* l_opnd);
extern Target* InvertTarget(Target* left_operand);

extern int_t   GetTargetLength(Target* seq);
extern Target* GetContentTarget(Target* sequence, int_t index);
extern Target* SliceTarget(Target* seq, int_t begin, int_t finish);

extern Target* TypeTarget(Target* l_opnd);

extern char_t  TargetToChar(Target* l_opnd);
extern int_t   TargetToInt(Target* l_opnd);
extern float_t TargetToFloat(Target* l_opnd);
extern char*   TargetToStr(Target* l_opnd);
extern bool    TargetToBool(Target* a);
extern Target* TargetToList(Target* l_opnd);

extern char_t  my_stoc(const char* str_ptr);
extern int_t   my_stoi(const char* str_ptr);
extern float_t my_stof(const char* str_ptr);

extern Target* TargetContentToStr(Target* target);


#ifdef _LANG_DEBUG_
void TargetFile_dump(FILE *fp);
void ConsoleTarget_Dump();
#endif  /* _LANG_DEBUG_ */

#endif
