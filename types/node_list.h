//
// Created by Legion on 06.12.2021.
//

#ifndef EXECORE_LIST_H
#define EXECORE_LIST_H

#include "target.h"

typedef struct list_target {
	TARGET_INFO_D;
	struct list_node* head;
	struct list_node* tail;
} TargetList;

typedef struct list_node {
	TARGET_INFO_D;
	struct list_node* next_list;
	struct list_node* prev_list;
	struct target*  target;
} NodeList;

typedef struct {
	TYPE_FUNC;

	Target* (*length)(TargetList* target);
	NodeList *(*item)(TargetList* str, int_t index);
	TargetList* (*slice)(TargetList* target, int_t start, int_t end);
	Target* (*cat)(TargetList* left_operand, TargetList* right_operand);
	Target* (*clone)(Target* left_operand, Target* right_operand);
	Target* (*equal)(TargetList* left_operand, TargetList* right_operand);
	Target* (*not_equal)(TargetList* left_operand, TargetList* right_operand);
	void (*insert)(TargetList* list, int_t index, Target* target);
	void (*append)(TargetList* list, Target* target);
	Target* (*remove)(TargetList* list, int_t index);
} ListType;

extern ListType ListT;

typedef struct {
    TYPE_FUNC;
} ListNodeType;

extern ListNodeType ListNodeT;

#define LIST_TARGET(o)	(((NodeList *)(o))->target)

#endif
