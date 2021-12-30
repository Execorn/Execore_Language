//
// Created by Legion on 04.12.2021.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "src/handle_err.h"
#include "target.h"
#include "src/number.h"
#include "null_target.h"
#include "node_list.h"
#include "execore_string.h"
#include "src/utils/macro_utils.h"


#ifdef _LANG_DEBUG_

static Target* head = NULL;
static Target* tail = NULL;

static void InsertToQueue(Target* target);
static void RemoveFromQueue(Target* target);

#define add_queue(o)  InsertToQueue(o)
#define remove_queue(o)  RemoveFromQueue(o)
#else
#define add_queue(o)  (() 0)
#define remove_queue(o)  (() 0)
#endif

Target* ConstructTarget(target_t type) {
	Target* target = NULL;

	switch (type) {
		case CHAR_T:
			target = charT.construct();
			break;

		case INT_T:
			target = intT.construct();
			break;

		case FLOAT_T:
			target = floatT.construct();
			break;

		case STR_T:
			target = StrT.construct();
			break;

		case LIST_T:
			target = ListT.construct();
			break;

		case LST_NODE_T:
			target = ListNodeT.construct();
			break;

		case NAN_T:
			target = NanT.construct();
			break;

        default:
            RaiseError(LANG_ERROR);
            break;
	}

	debug_printf(MEM_DEBUG, "\n alloc : %-p", (void*) target);

	if (!target) RaiseError(MEM_ERROR);
	else {
		debug_printf(MEM_DEBUG, " %s", TARGET_NAME(target));
        add_queue(target);
        IncreaseUsages(target);
	}
	return target;
}

Target* MakeTarget(target_t type, ...) {
	va_list va_args;
	Target* target;

	va_start(va_args, type);

	if ((target = ConstructTarget(type)) != NULL) {
        TARGET_TYPE(target)->adv_init(target, va_args);
    }

	va_end(va_args);

	return target;
}

void FreeTarget(Target* target) {
    REPORT(target);

	remove_queue(target);
	debug_printf(MEM_DEBUG, "\nfree  : %-p %s", (void*) target, TARGET_NAME(target));

	TARGET_TYPE(target)->free(target);
}

Target* GetTargetMethod(Target* target, char* method, Array* arguments) {
    REPORT(target);
    REPORT(method);
    REPORT(arguments);

	return TARGET_TYPE(target)->method(target, method, arguments);
}

void DumpTarget(FILE* dump_file, Target* cur_trg) {
    REPORT(cur_trg);

	TARGET_TYPE(cur_trg)->dump(dump_file, cur_trg);
}

Target* GetTarget(FILE* dump_file, target_t type) {
	char buffer[MAX_LINE_LENGTH] = "";
	Target* target = NULL;

	if (fgets(buffer, sizeof(buffer), dump_file) == NULL) {
		buffer[0] = 0;
	}
	buffer[strcspn(buffer, "\r\n")] = 0;

	switch (type) {
		case CHAR_T:
			target = MakeTarget(CHAR_T, my_stoc(buffer));
			break;
            
		case INT_T:
			target = MakeTarget(INT_T, my_stoi(buffer));
			break;
            
		case FLOAT_T:
			target = MakeTarget(FLOAT_T, my_stof(buffer));
			break;
            
		case STR_T:
			target = MakeTarget(STR_T, buffer);
			break;
            
		default:
            RaiseError(TYPE_ERROR, "I will not take this shit for input: %d", type);
			target = ConstructTarget(NAN_T);
	}

	return target;
}

Target* CopyTarget(Target* cur_trg) {
    REPORT(cur_trg);
    
	switch (TYPE(cur_trg)) {
		case CHAR_T:
			return MakeTarget(CHAR_T, TargetToChar(cur_trg));

		case INT_T:
			return MakeTarget(INT_T, TargetToInt(cur_trg));

		case FLOAT_T:
			return MakeTarget(FLOAT_T, TargetToFloat(cur_trg));

		case STR_T:
			return MakeTarget(STR_T, TargetToStr(cur_trg));

		case LIST_T:
			return MakeTarget(LIST_T, TargetToList(cur_trg));

		case LST_NODE_T:
			return CopyTarget(LIST_TARGET(cur_trg));

		default:
            RaiseError(TYPE_ERROR, "noooo my copy machine broke :(((( %s", TARGET_NAME(cur_trg));
			return ConstructTarget(NAN_T);
	}
}

void AssignTarget(Target* cur_trg, Target* next_trg) {
    REPORT(cur_trg);
    REPORT(next_trg);
    
	Target* target = NULL;

	switch (TYPE(cur_trg)) {
		case CHAR_T:
			TARGET_TYPE(cur_trg)->init(cur_trg, TargetToChar(next_trg));
			break;

		case INT_T:
			TARGET_TYPE(cur_trg)->init(cur_trg, TargetToInt(next_trg));
			break;

		case FLOAT_T:
			TARGET_TYPE(cur_trg)->init(cur_trg, TargetToFloat(next_trg));
			break;

		case STR_T:
			target = TargetContentToStr(next_trg);
			TARGET_TYPE(cur_trg)->init(cur_trg, TargetToStr(target));
            DecreaseUsages(target);
			break;

		case LIST_T:
			TARGET_TYPE(cur_trg)->init(cur_trg, TargetToList(next_trg));
			break;

		case LST_NODE_T:
			TARGET_TYPE(cur_trg)->init(cur_trg, CopyTarget(next_trg));
			break;

		default:
            RaiseError(TYPE_ERROR, "wtf is this type(s) for assignment? %s and %s",
                       TARGET_NAME(cur_trg), TARGET_NAME(next_trg));
	}
}

Target* AddTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
	right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.add(left_operand, right_operand);
    } else if (IS_STR(left_operand) || IS_STR(right_operand)) {
        return StrT.cat(left_operand, right_operand);
    } else if (IS_LIST_T(left_operand) && IS_LIST_T(right_operand)) {
        return ListT.cat((TargetList *)left_operand, (TargetList *)right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for adding? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* SubTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  : left_operand;
	right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.sub(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for subbing? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* MulTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
	right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.mul(left_operand, right_operand);
    } else if ((IS_NUM(left_operand) || IS_NUM(right_operand)) && (IS_STR(left_operand) || IS_STR(right_operand))) {
        return StrT.clone(left_operand, right_operand);
    } else if ((IS_NUM(left_operand) || IS_NUM(right_operand)) &&
              (IS_LIST_T(left_operand) || IS_LIST_T(right_operand))) {
        return ListT.clone(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for mul? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* DivTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
	right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.div(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for div: %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* ModTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.mod(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "unsupported operand type(s) for operation %%: %s and %s", \
                          TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* InvertTarget(Target* left_operand) {
    REPORT(left_operand);

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	if (IS_NUM(left_operand)) {
        return numT.invert(left_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for inverting: %s",
                   TARGET_NAME(left_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* EqualTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.equal(left_operand, right_operand);
    } else if (IS_STR(left_operand) && IS_STR(right_operand)) {
        return StrT.equal(left_operand, right_operand);
    } else if (IS_LIST_T(left_operand) && IS_LIST_T(right_operand)) {
        return ListT.equal((TargetList*) left_operand, (TargetList*) right_operand);
    } else return MakeTarget(INT_T, 0);
}

Target* NotEqualTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.not_equal(left_operand, right_operand);
    } else if (IS_STR(left_operand) && IS_STR(right_operand)) {
        return StrT.not_equal(left_operand, right_operand);
    } else if (IS_LIST_T(left_operand) && IS_LIST_T(right_operand)) {
        return ListT.not_equal((TargetList*) left_operand, (TargetList*) right_operand);
    } else return MakeTarget(INT_T, 1);
}

Target* LessTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.less_t(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for op less? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* LessEqualTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.less_equal_t(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for op <=? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* GreaterTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.greater_t(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for op >? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* GreaterEqualTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.greater_equal_t(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for >=? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* LogOrTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.or(left_operand, right_operand);
    }
    else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for op 'or'? %s and %s",
                   TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* LogAndTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

    if (IS_NUM(left_operand) && IS_NUM(right_operand)) {
        return numT.and(left_operand, right_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for op 'and'? %s and %s",
                          TARGET_NAME(left_operand), TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* LogInTarget(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    Target* result      = NULL;
	Target* constructor = NULL;
	int_t constr_len    = 0;

    left_operand  = IS_LIST(left_operand)  ? LIST_TARGET(left_operand)  :  left_operand;
    right_operand = IS_LIST(right_operand) ? LIST_TARGET(right_operand) : right_operand;

	if (!IS_SEQ(right_operand)) {
        RaiseError(TYPE_ERROR, "shit, I can't take index of %s", TARGET_NAME(right_operand));
		return ConstructTarget(NAN_T);
	}
    constr_len = GetTargetLength(right_operand);

	for (int_t i = 0; i < constr_len; i++) {
		if (result) DecreaseUsages(result);

        constructor = GetContentTarget(right_operand, i);
		result = EqualTarget(left_operand, constructor);
        DecreaseUsages(constructor);

		if (TargetToInt(result) == 1) break; // we got a result, it is IN!
	}
	return result;
}

Target* NegateTarget(Target* left_operand) {
    REPORT(left_operand);

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	if (IS_NUM(left_operand)) {
        return numT.negate(left_operand);
    } else {
        RaiseError(TYPE_ERROR, "wtf is this type(s) for !? %s and %s", TARGET_NAME(left_operand));
		return ConstructTarget(NAN_T);
	}
}

Target* GetContentTarget(Target* seq, int_t index) {
    REPORT(seq);
    
	seq = IS_LIST(seq) ? LIST_TARGET(seq) : seq;

	if (TYPE(seq) == STR_T) {
        return (Target*) StrT.content((TargetStr *)seq, index);
    }
	else if (TYPE(seq) == LIST_T)
		return (Target*) ListT.item((TargetList *)seq, index);
	else {
        RaiseError(TYPE_ERROR, "shit, I can't take index of %s", TARGET_NAME(seq));
		return ConstructTarget(NAN_T);
	}
}

Target* SliceTarget(Target* seq, int_t begin, int_t finish) {
    REPORT(seq);

    seq = IS_LIST(seq) ? LIST_TARGET(seq) : seq;

	if (TYPE(seq) == STR_T) {
        return (Target*) StrT.slice((TargetStr *)seq, begin, finish);
    } else if (TYPE(seq) == LIST_T) {
        return (Target*) ListT.slice((TargetList*) seq, begin, finish);
    } else {
        RaiseError(TYPE_ERROR, "shit, I can't take index of %s", TARGET_NAME(seq));
		return ConstructTarget(NAN_T);
	}
}

int_t GetTargetLength(Target* seq) {
    REPORT(seq);

	int_t seq_len  =    0;
	Target* target = NULL;

	seq = IS_LIST(seq) ? LIST_TARGET(seq) : seq;

	if (TYPE(seq) == STR_T) {
        target = StrT.len((TargetStr *)seq);
    } else if (TYPE(seq) == LIST_T) {
        target = ListT.length((TargetList *)seq);
    } else RaiseError(TYPE_ERROR, "shit, I can't take index of %s", TARGET_NAME(seq));

	if (target) {
        seq_len = TargetToInt(target);
        DecreaseUsages(target);
	}
	return seq_len;
}

Target* TypeTarget(Target* left_operand) {
    REPORT(left_operand);

	Target* result = NULL;
	result = MakeTarget(STR_T, TARGET_NAME(left_operand));

	if (!result) result = ConstructTarget(NAN_T);

	return result;
}

char_t TargetToChar(Target* left_operand) {
    REPORT(left_operand);

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	switch (TYPE(left_operand)) {
		case CHAR_T:
			return ((TargetChar*) left_operand)->char_value;

		case INT_T:
			return (char_t) ((TargetInt*) left_operand)->int_value;

		case FLOAT_T:
			return (char_t) ((TargetFloat*) left_operand)->float_value;

		case STR_T:
			return my_stoc(((TargetStr *) left_operand)->string_ptr);

		default:
            RaiseError(VALUE_ERROR, "this shit is not converting to char :( - help me with %s", TARGET_NAME(left_operand));
			return 0;
	}
}

int_t TargetToInt(Target* left_operand) {
    REPORT(left_operand);

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	switch (TYPE(left_operand)) {
		case CHAR_T:
			return ((TargetChar*) left_operand)->char_value;

		case INT_T:
			return ((TargetInt *)left_operand)->int_value;

		case FLOAT_T:
			return (int_t) ((TargetFloat*) left_operand)->float_value;

		case STR_T:
			return my_stoi(((TargetStr*) left_operand)->string_ptr);

		default:
            RaiseError(VALUE_ERROR, "this shit is not converting to integer :( - help me with %s",
                       TARGET_NAME(left_operand));
			return 0;
	}
}

float_t TargetToFloat(Target* left_operand) {
    REPORT(left_operand);

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	switch (TYPE(left_operand)) {
		case CHAR_T:
			return ((TargetChar*) left_operand)->char_value;

		case INT_T:
			return (float_t) ((TargetInt*) left_operand)->int_value;

		case FLOAT_T:
			return ((TargetFloat*) left_operand)->float_value;

		case STR_T:
			return my_stof(((TargetStr *) left_operand)->string_ptr);

		default:
            RaiseError(VALUE_ERROR, "this shit is not converting to float :( - help me with %s",
                       TARGET_NAME(left_operand));
			return 0;
	}
}

char* TargetToStr(Target* left_operand) {
    REPORT(left_operand);

	static char* empty = "";

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	switch (TYPE(left_operand)) {
		case STR_T:
            return ((TargetStr*) left_operand)->string_ptr;

		default:
            RaiseError(VALUE_ERROR, "this shit is not converting to string :( - help me with %s",
                       TARGET_NAME(left_operand));
			return empty;
	}
}

Target* TargetToList(Target* left_operand) {
    REPORT(left_operand);

	Target* target = NULL;

    left_operand = IS_LIST(left_operand) ? LIST_TARGET(left_operand) : left_operand;

	switch (TYPE(left_operand)) {
		case LIST_T:
			return left_operand;

		default:
            RaiseError(VALUE_ERROR, "this shit is not converting to list :( - help me with %s",
                       TARGET_NAME(left_operand));
			if ((target = ConstructTarget(LIST_T)) != NULL) {
                return target;
            } else return ConstructTarget(NAN_T);
	}
}

/* auto-cast to bool in return */
bool TargetToBool(Target* cur_trg) {
    REPORT(cur_trg);

    cur_trg = IS_LIST(cur_trg) ? LIST_TARGET(cur_trg) : cur_trg;

	switch (TYPE(cur_trg)) {
		case CHAR_T:
			return TargetToChar(cur_trg);

		case INT_T:
			return TargetToInt(cur_trg);

		case FLOAT_T:
			return (bool) TargetToFloat(cur_trg);

		default:
            RaiseError(VALUE_ERROR, "this shit is not converting to bool :( - help me with %s",
                       TARGET_NAME(cur_trg));
			return false;
	}
}

char_t my_stoc(const char* str_ptr) {
    REPORT(str_ptr);

	char_t symbol = 0;

    /* checking for escape sequence first */
	if (*str_ptr == '\\') {
		switch (*++str_ptr) {
			case '0' :
                symbol = '\0';
                break;

			case 'b' :
                symbol = '\b';
                break;

			case 'f' :
                symbol = '\f';
                break;

			case 'n' :
                symbol = '\n';
                break;

			case 'r' :
                symbol = '\r';
                break;

			case 't' :
                symbol = '\t';
                break;

			case 'v' :
                symbol = '\v';
                break;

			case '\\':
                symbol = '\\';
                break;

			case '\'':
                symbol = '\'';
                break;

			case '\"':
                symbol = '\"';
                break;

            default:
                RaiseError(VALUE_ERROR, "wtf is this escape sequence? %symbol", *str_ptr);
                return 0; // return 0 because numbers cannot contain escape sequences
		}
	} else {
		if (*str_ptr == '\0') {
            RaiseError(SYNTAX_ERROR, "stop sending me this this. NO EMPTY CHAR CONSTANTS pls...");
			return 0;
		} else symbol = *str_ptr;
	}
	if (*++str_ptr != '\0') { // num does not end
        RaiseError(SYNTAX_ERROR, "bro there is too many chars in your constant");
		return 0;
	}

	return symbol;
}

int_t my_stoi(const char* str_ptr) {
    REPORT(str_ptr);

	char* old_ptr = 0;
	int_t value   = 0;

	errno         = 0; // to handle errors and use strerror()

	value = (int_t) strtol(str_ptr, &old_ptr, 10);

    /* if old_ptr = str_ptr, then num has 0 GetListLen lol... */

	if (old_ptr == str_ptr || errno) {
		if (errno) {
            RaiseError(VALUE_ERROR, "help wtf is this? how do I convert it to int? - %s; %s",
                       str_ptr, strerror(errno));
        } else {
            RaiseError(VALUE_ERROR, "help wtf is this? how do I convert it to int? - %s", str_ptr);
        }
		return 0;
	} else
		return value;
}

float_t my_stof(const char* str_ptr) {
    REPORT(str_ptr);

	char* old_ptr = 0;
	float_t new_float = 0;

	errno = 0;

    new_float = (float_t) strtod(str_ptr, &old_ptr);

	if (old_ptr == str_ptr || errno) {
		if (errno) {
            RaiseError(VALUE_ERROR, "help wtf is this? how do I convert it to float? - %s; %s",
                               str_ptr, strerror(errno));
        } else {
            RaiseError(VALUE_ERROR, "help wtf is this? how do I convert it to int? - %s", str_ptr);
        }
		return 0;
	} else
		return new_float;
}

Target* TargetContentToStr(Target* target) {
    REPORT(target);

	char buffer[MAX_NUM_LENGTH] = "";

	switch (TYPE(target)) {
		case STR_T:
            IncreaseUsages(target);
			return target;

		case CHAR_T:
			snprintf(buffer, sizeof(buffer), "%c", TargetToChar(target));
			return MakeTarget(STR_T, buffer);

		case INT_T:
			snprintf(buffer, sizeof(buffer), "%ld", TargetToInt(target));
			return MakeTarget(STR_T, buffer);

		case FLOAT_T:
			snprintf(buffer, sizeof(buffer), "%.16lG", TargetToFloat(target));
			return MakeTarget(STR_T, buffer); /* округление до 16 знаков после запятой, нули не добавляются */
            /* l is used for wide chars or strings */

		case NAN_T:
			return MakeTarget(STR_T, "NONE");

		default:
			return MakeTarget(STR_T, "");
	}
}


#ifdef _LANG_DEBUG_

static void InsertToQueue(Target* target) {
    REPORT(target);

	if (!head) { // if head has default value
		head = target;
        target->prev = NULL;
	} else {
        target->prev = tail;
		tail->next   = target;
	}
	tail = target;
    target->next = NULL;
}

static void RemoveFromQueue(Target* target) {
	if (target->next == NULL) {
		if (target->prev == NULL) {
			head = tail = NULL;
		} else {
			tail = target->prev;
			tail->next = NULL;
		}
	} else {
		if (target->prev == NULL) {
			head = target->next;
			head->prev = NULL;
		} else {
            target->prev->next = target->next;
            target->next->prev = target->prev;
		}
	}
}

void TargetFile_dump(FILE* dump_file) {
	fprintf(dump_file, "%s - %s - %s - %s\n", "target_id", "usages", "type", "val");

	for (Target* target = head; target; target = target->next) {
		fprintf(dump_file, "%-p - %d - %s ", (void*) target, target->usages, TARGET_NAME(target));
        DumpTarget(dump_file, target);
		fprintf(dump_file, "\n");
	}
}

void ConsoleTarget_Dump() {
	FILE* dump_file = NULL;

	if ((dump_file = fopen("id_targets.dsv", "w")) != NULL) {
        TargetFile_dump(dump_file);
		fclose(dump_file);
	}
}
#endif  /* _LANG_DEBUG_ */
