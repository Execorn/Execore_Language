//
// Created by Legion on 09.12.2021.
//

#ifndef EXECORE_ID_H
#define EXECORE_ID_H

#include "src/types/target.h"

typedef enum {
    VARIABLE = 1,
    FUNCTION,
} identifier_t;

static inline char* IdTypeToString(identifier_t id_type) {
	static char* names[] = {"?", "VARIABLE", "FUNCTION"};

	return names[id_type < 0 || id_type > sizeof(names) / sizeof(names[0])];
}

#include "src/utils/struct_info.h"
typedef struct identifier {
	ID_INFO;

	OBJ_AND_NODE;

	ID_FUNC;
} Identifier;

extern Identifier identifier;

typedef struct scope {
    SCOPE_INFO;

    void (*add_child   )(bool is_nested);
	void (*remove_child)();
} Scope;

extern Scope main_scope;


#ifdef _LANG_DEBUG_
void FileID_Dump(FILE* stream);
void ConsoleID_Dump();
#endif  /* _LANG_DEBUG_ */

#endif

