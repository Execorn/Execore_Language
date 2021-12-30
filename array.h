//
// Created by Legion on 07.12.2021.
//

#ifndef EXECORE_ARRAY_H
#define EXECORE_ARRAY_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct array {
	size_t current_index;
	void** data;
} Array;

Array* ArrayConstructor();
void   ArrayDestructor(Array* arr);

extern bool AppendNode(Array* arr, void* node);
extern bool InsertNode(Array* arr, size_t prev, void* node);
extern bool RemoveNode(Array* arr, size_t idx);

#endif
