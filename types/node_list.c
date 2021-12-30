//
// Created by Legion on 06.12.2021.
//

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "target.h"
#include "src/handle_err.h"
#include "src/processor.h"
#include "node_list.h"
#include "src/utils/macro_utils.h"

static const TargetList EMPTY_TARGET   = (const TargetList) {0};
static const NodeList   EMPTY_NODELIST = (const NodeList)   {0};

static int_t GetListLen(TargetList* target);

static TargetList* ConstructList() {
	TargetList* target = NULL;

	if ((target = calloc(1, sizeof(TargetList))) != NULL) {
		target->target_type = (TargetType*) &ListT;
		target->type        = LIST_T;
		target->usages      = 0;

		target->head     = NULL;
		target->tail     = NULL;
	}
	return target;
}

static void FreeList(TargetList* target) {
    REPORT(target);

	Target* item = NULL;

	while (GetListLen(target) > 0) {
		item = ListT.remove(target, 0);
        DecreaseUsages(item);
	}

	*target = EMPTY_TARGET;
	free(target);
}

static void PrintList(FILE* dump_file, TargetList* target) {
    REPORT(dump_file);
    REPORT(target);

	printf("[");
	for (NodeList* list_node = target->head; list_node; list_node = list_node->next_list) {
        DumpTarget(dump_file, list_node->target);
		if (list_node->next_list)
			fprintf(dump_file, ",");
	}
	fprintf(dump_file, "]");
}

static void CopyList(TargetList* to_list, TargetList* from_list) {
    REPORT(to_list);
    REPORT(from_list);

    Target* item = NULL;

	while (GetListLen(to_list) > 0) {
		item = ListT.remove(to_list, 0);
        DecreaseUsages(item);
	}

	for (NodeList* list_node = from_list->head; list_node; list_node = list_node->next_list) {
        ListT.append(to_list, CopyTarget(list_node->target));
    }
}


static void ListAdv_init(TargetList* target, va_list va_args) {
    REPORT(target);

    CopyList(target, va_arg(va_args, TargetList*));
}

static Target* GetListMethod(TargetList* target, char* name, Array* arguments) {
    REPORT(target);
    REPORT(name);
    REPORT(arguments);

	Target* result     = NULL;
	Stack* constructor = NULL;

    constructor = StackConstructor(10);

	if (!strcmp("len", name)) {
		if (arguments->current_index != 0) {
            RaiseError(SYNTAX_ERROR, "give %s %d args pls", name, 2);
			result = ConstructTarget(NAN_T);
		} else result = ListT.length(target);
	} else if (!strcmp("insert", name)) {
		if (arguments->current_index != 2) {
            RaiseError(SYNTAX_ERROR, "give %s %d args pls", name, 2);
        } else {
            CompileNode(arguments->data[0], constructor);
			Target* act_target = StackPop(constructor);

            CompileNode(arguments->data[1], constructor);
			Target* value = StackPop(constructor);
			ListT.insert(target, TargetToInt(act_target), CopyTarget(value));

            DecreaseUsages(value);
            DecreaseUsages(act_target);
		}
		result = ConstructTarget(NAN_T);
	} else if (!strcmp("append", name)) {
		if (arguments->current_index != 1) {
            RaiseError(SYNTAX_ERROR, "give %s %d args pls", name, 1);
        } else {
            CompileNode(arguments->data[0], constructor);
			Target* value = StackPop(constructor);
			ListT.append(target, CopyTarget(value));

            DecreaseUsages(value);
		}
		result = ConstructTarget(NAN_T);
	} else if (!strcmp("remove", name)) {
		if (arguments->current_index != 1) {
            RaiseError(SYNTAX_ERROR, "give %s %d args pls", name, 1);
			result = ConstructTarget(NAN_T);
		} else {
            CompileNode(arguments->data[0], constructor);
			Target* index = StackPop(constructor);
            result = ListT.remove(target, TargetToInt(index));

            DecreaseUsages(index);
		}
	} else {
        RaiseError(SYNTAX_ERROR, "where did u find method %s in %s?", TARGET_NAME(target), name);
		result = ConstructTarget(NAN_T);
	}

    StackDestructor(constructor);

	return result;
}

static int_t GetListLen(TargetList* target) {
    REPORT(target);

	NodeList* list_node = NULL;
	int_t i = 0;

	for (i, list_node = target->head; list_node; i++, list_node = list_node->next_list);
	return i;
}

static Target* GetLenTarget(TargetList* target) {
    REPORT(target);

	Target* target_len = NULL;

	if ((target_len = MakeTarget(INT_T, GetListLen(target))) == NULL) {
        target_len = ConstructTarget(NAN_T);
    }

	return target_len;
}

static Target* ListConcatenate(TargetList* left_operand, TargetList* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	TargetList* list = NULL;
	NodeList*   item = NULL;
	int_t       i    =    0;

	if ((list = (TargetList*) ConstructTarget(LIST_T)) == NULL) {
        return ConstructTarget(NAN_T);
    }

	for (i = 0; i < GetListLen(left_operand); i++) {
		item = ListT.item(left_operand, i);
		ListT.append(list, CopyTarget(item->target));
        DecreaseUsages(item);
	}

	for (i = 0; i < GetListLen(right_operand); i++) {
		item = ListT.item(right_operand, i);
		ListT.append(list, CopyTarget(item->target));
        DecreaseUsages(item);
	}

	return (Target*) list;
}

static Target* DuplicateList(Target* left_operand, Target* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    TargetList* list = NULL;
    NodeList*   item = NULL;
    int_t       i    =    0;
    int_t copies     =    0;

	Target* old_list = TYPE(left_operand) == LIST_T ? left_operand : right_operand;
 	Target* new_list = TYPE(left_operand) == LIST_T ? right_operand : left_operand;

    copies = TargetToInt(new_list);

	if (copies < 0) copies = !copies;

	if ((list = (TargetList*) ConstructTarget(LIST_T)) == NULL) {
        return ConstructTarget(NAN_T);
    }
	while (copies--) {
        for (i = 0; i < GetListLen((TargetList *) old_list); i++) {
            item = ListT.item((TargetList*)old_list, i);
            ListT.append(list, CopyTarget(item->target));
            DecreaseUsages(item);
        }
    }

	return (Target*) list;
}

static bool CompareLists(TargetList* left_operand, TargetList* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

    bool is_equal    =    0;
	Target* target   = NULL;
	int_t i          =    0;
    int_t first_list =    0;
	NodeList* item1  = NULL;
    NodeList* item2  = NULL;

    first_list = GetListLen(left_operand);

	if (first_list != GetListLen(right_operand)) {
        return false;
    }
	for (is_equal = true, i = 0; i < first_list; i++) {
		item1 = ListT.item(left_operand, i);
		item2 = ListT.item(right_operand, i);
		target = EqualTarget((Target*) item1, (Target*) item2);
        is_equal = TargetToBool(target);
        DecreaseUsages(item1);
        DecreaseUsages(item2);
        DecreaseUsages(target);
		if (!is_equal) break;
	}
	return i == first_list;
}

static Target* EqualList(TargetList* left_operand, TargetList* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	Target* result = NULL;

	if ((result = MakeTarget(INT_T, CompareLists(left_operand, right_operand))) == NULL) {
        result = ConstructTarget(NAN_T);
    }

	return result;
}

static Target* NotEqualList(TargetList* left_operand, TargetList* right_operand) {
    REPORT(left_operand);
    REPORT(right_operand);

	Target* result = NULL;

	if ((result = MakeTarget(INT_T, !CompareLists(left_operand, right_operand))) == NULL) {
        result = ConstructTarget(NAN_T);
    }

	return result;
}

static NodeList* GetListItem(TargetList* list, int_t index) {
    REPORT(list);

	NodeList* list_node  = NULL;
	int_t cur_len = 0, i =    0;

    cur_len = GetListLen(list);

	if (index < 0) index += cur_len;

	if (index < 0 || index >= cur_len) {
        RaiseError(INDEX_ERROR);
		return (NodeList*) ConstructTarget(NAN_T);
	}

	for (i = 0, list_node = list->head; list_node; i++, list_node = list_node->next_list) {
		if (i == index) break;
	}

    IncreaseUsages(list_node);

	return list_node;
}

static TargetList* GetListSlice(TargetList* list, int_t start, int_t end) {
    REPORT(list);

	TargetList* slice   = NULL;
	NodeList* list_node = NULL;
	int_t cur_len       =    0;

    cur_len = GetListLen(list);

	if (start < 0) start += cur_len;
	if (end < 0) end += cur_len;
	if (start < 0) start = !start;
	if (end >= cur_len) end = cur_len;


	if ((slice = (TargetList*) ConstructTarget(LIST_T)) != NULL) {
		for (int_t i = start; i < end; i++) {
            list_node = ListT.item(list, i);
			ListT.append(slice, CopyTarget(list_node->target));
            DecreaseUsages(list_node);
		}
	} else slice = (TargetList*) ConstructTarget(NAN_T);

	return slice;
}

static void ListAppend(TargetList* target_list, Target* target) {
    REPORT(target_list);
    REPORT(target);

    NodeList* list_node = NULL;
    NodeList* tail      = NULL;

	if ((list_node = (NodeList*) MakeTarget(LST_NODE_T, target)) == NULL) return;

	if (!target_list->head) {
		target_list->head = list_node;
        target_list->tail = list_node;
	} else {
		tail                 = target_list->tail;
        list_node->prev_list      = tail;
		tail->next_list           = list_node;
        target_list->tail    = list_node;
	}
}

static void ListInsert(TargetList* target_list, int_t index, Target* target) {
    REPORT(target_list);
    REPORT(target);

	NodeList* list_node = NULL;
    NodeList* new_ptr   = NULL;

	int_t cur_len       =    0;

	if ((list_node = (NodeList*) MakeTarget(LST_NODE_T, target)) == NULL) return;

	if (!target_list->head) {
		target_list->head = list_node;
        target_list->tail = list_node;
	} else {
		cur_len = GetListLen(target_list);
		if (index < 0) index += cur_len;

		if (index <= 0) {
			list_node->next_list         = target_list->head;
			list_node->next_list->prev_list   = list_node;
			list_node->prev_list         = NULL;
            target_list->head       = list_node;
		} else if (index >= cur_len) {
			new_ptr = target_list->tail;
			list_node->prev_list = new_ptr;
            new_ptr->next_list = list_node;
            target_list->tail = list_node;
		} else {
			for (new_ptr = target_list->head; index--; new_ptr = new_ptr->next_list);
			list_node->next_list              = new_ptr;
			list_node->prev_list              = new_ptr->prev_list;
			list_node->next_list->prev_list   = list_node;
			list_node->prev_list->next        = (Target*) list_node;
		}
	}
}

static Target* ListRemove(TargetList* target_list, int_t idx) {
    REPORT(target_list);

	NodeList* list_node = NULL;
	Target* target      = NULL;
	int_t cur_len       =    0;
    int_t index         =    0;

    cur_len = GetListLen(target_list);
	if (idx < 0) idx += cur_len;

	if (idx < 0 || idx >= cur_len) {
        return ConstructTarget(NAN_T);
    }

	for (index = 0, list_node = target_list->head; list_node; index++, list_node = list_node->next_list) {
		if (index == idx) {
			target = list_node->target;

			if (target_list->head == target_list->tail) {
				target_list->head = NULL;
                target_list->tail = NULL;
			} else if (!list_node->prev_list) {
				target_list->head = list_node->next_list;
				list_node->next_list->prev_list = NULL;
			} else if (!list_node->next_list) {
				target_list->tail = list_node->prev_list;
				list_node->prev_list->next = NULL;
			} else {
				list_node->prev_list->next      = (Target*) list_node->next_list;
				list_node->next_list->prev_list = list_node->prev_list;
			}

            IncreaseUsages(target);
            DecreaseUsages(list_node);
			break;
		}
	}
	return target;
}

ListType ListT = {
	.name       = "list",
	.construct  = (Target* (*)()) ConstructList,
	.free       = (void (*)(Target*))  FreeList,
	.dump       = (void (*)(FILE *, Target* )) PrintList,
	.init       = CopyList,
	.adv_init   = (void (*)(Target* , va_list)) ListAdv_init,
	.method     = (Target* (*)(Target* , char* , Array* )) GetListMethod,

	.length     = GetLenTarget,
	.item       = GetListItem,
	.slice      = GetListSlice,
	.cat        = ListConcatenate,
	.clone      = DuplicateList,
	.equal      = EqualList,
	.not_equal  = NotEqualList,
	.insert     = ListInsert,
	.append     = ListAppend,
	.remove     = ListRemove,
};

static NodeList* ConstructNodeList() {
	NodeList* target = NULL;

	if ((target = calloc(1, sizeof(NodeList))) != NULL) {
		target->target_type      = (TargetType *)&ListNodeT;
		target->type         = LST_NODE_T;
		target->usages     = 0;

		target->next_list         = NULL;
		target->prev_list         = NULL;
		target->target       = NULL;
	}
	return target;
}

static void FreeListNode(NodeList* list_node) {
    REPORT(list_node);

	if (list_node->target) {
        DecreaseUsages(list_node->target);
    }
	*list_node = EMPTY_NODELIST;

	free(list_node);
}

static void PrintNodeList(FILE* dump_file, NodeList* list_node) {
    REPORT(dump_file);
    REPORT(list_node);

    DumpTarget(dump_file, list_node->target);
}


static void ListNodeInit(NodeList* list_node, Target* target) {
    REPORT(list_node);
    REPORT(target);

	if (list_node->target) {
        DecreaseUsages(list_node->target);
    }
	list_node->target = target; // make a target
}


static void ListNodeAdv_init(NodeList* list_node, va_list va_args) {
    REPORT(list_node);

    ListNodeInit(list_node, va_arg(va_args, Target*));
}


static NodeList* ListNodeMethodError(NodeList* target, char* name, Array* arguments) {
    REPORT(target);
    REPORT(name);
    REPORT(arguments);

	NAEBAL_GCC(arguments);

    RaiseError(SYNTAX_ERROR, "where the hell did u find method %s in %s?", TARGET_NAME(target), name);

	return (NodeList*) ConstructTarget(NAN_T);
}

ListNodeType ListNodeT = {
	.name        = "list_node",
	.construct   = (Target* (*)()) ConstructNodeList,
	.free        = (void (*)(Target*)) FreeListNode,
	.dump        = (void (*)(FILE *, Target* )) PrintNodeList,
	.init        = ListNodeInit,
	.adv_init    = (void (*)(Target* , va_list)) ListNodeAdv_init,
	.method      = (Target* (*)(Target* , char* , Array* )) ListNodeMethodError,
};
