//
// Created by Legion on 07.12.2021.
//

#include <stdbool.h>
#include <string.h>

#include "array.h"
#include "handle_err.h"
#include "src/utils/macro_utils.h"


Array* ArrayConstructor() {
	Array* new_arr = NULL;

	if ((new_arr = malloc(sizeof(Array))) == NULL) {
        RaiseError(MEM_ERROR);
    } else {
        new_arr->current_index = 0;
        new_arr->data = NULL;
	}
	return new_arr;
}

void ArrayDestructor(Array* arr) {
    REPORT(arr); // AST_byPass is important not to get segfault!! (for example, if ArrDtor will be used 2 times)

	if (arr->data) {
        free(arr->data);
    }
	free(arr);
}

bool AppendNode(Array* arr, void* node) {
    REPORT(arr);

	void* new_data = NULL;

	if (!arr->current_index) {
		if ((arr->data = calloc(++arr->current_index /* ++arr->current_index = 1 */, sizeof(void*))) == NULL) {
            RaiseError(MEM_ERROR);
			return false;
		}
	} else {
		if ((new_data = realloc(arr->data, ++arr->current_index * sizeof(void*))) == NULL) {
            RaiseError(MEM_ERROR);
			return false;
		}
        arr->data = new_data;
	}
    arr->data[arr->current_index - 1] = node;
	return true;
}

bool RemoveNode(Array* arr, size_t idx) {
    REPORT(arr);

	void* new_data = NULL;

	if (idx >= arr->current_index) return false; // nothing to remove, INDEX ERROR

	memmove(arr->data + idx, arr->data + idx + 1, (arr->current_index + 1 - idx) * sizeof(void*));

	if ((new_data = realloc(arr->data, --arr->current_index * sizeof(void*))) == NULL) {
        return false;
    }
    arr->data = new_data;

	return true;
}

bool InsertNode(Array* arr, size_t prev, void* node) {
    REPORT(arr);

	void* new_data = NULL;
	if (prev >= arr->current_index) return false; // array overflow, INDEX ERROR

	if ((new_data = realloc(arr->data, ++arr->current_index * sizeof(void*))) == NULL) {
        return false;
    }
    arr->data = new_data;

    unsigned size_to_move = (arr->current_index + 1 - prev) * sizeof(void *);
	memmove(arr->data + prev + 1, arr->data + prev, size_to_move);
    arr->data[prev] = node;

	return true;
}
