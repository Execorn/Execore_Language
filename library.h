//
// Created by Legion on 02.12.2021.
//

#ifndef EXECORE_LIBRARY_H
#define EXECORE_LIBRARY_H

#include "src/utils/struct_info.h"
typedef struct lib {
    /* buf_line_index is index of current line's beginning in buffer */
    /* buf_char_index is index of current char in buffer             */
	LIB_INFO;

	CHAR_FUNC;

    LIB_FUNC;
} library;

extern library current_library;

#endif
