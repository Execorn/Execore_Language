//
// Created by Legion on 09.12.2021.
//

#include <stdlib.h>
#include <string.h>

#include "id.h"
#include "handle_err.h"


static Scope default_scope = {NULL, NULL, false};
const  Identifier NULL_ID  = (Identifier) {0};

static Scope* global_scope = &default_scope;
static Scope* local_scope  = &default_scope;


static Identifier* ScopeSearch(const Scope* actual_scope, const char* id_name) {
	Identifier* current_id = NULL;

	for (current_id = actual_scope->first_id; current_id != NULL; current_id = current_id->next_id) {
        if (!strcmp(id_name, current_id->name)) break;
    }

	return current_id;
}

static Identifier* search(const char* name) {
	Scope* actual_scope = local_scope;
	Identifier* req_id = NULL;

	while (true) {
		if ((req_id = ScopeSearch(actual_scope, name)) == NULL) {
			if (actual_scope->is_nested && actual_scope->higher_scope) {
                actual_scope = actual_scope->higher_scope;
				continue;
			} else req_id = ScopeSearch(global_scope, name);
		}
		break;
	}

	return req_id;
}


static Identifier* MakeId(identifier_t type, Scope* actual_scope, const char* id_name) {
    // actual_scope and id_name != NULL is guaranteed

	Identifier* new_id = NULL;

	if (!(ScopeSearch(actual_scope, id_name))) {
		if ((new_id = calloc(1, sizeof(Identifier))) == NULL) {
            RaiseError(MEM_ERROR);
        }
		*new_id = identifier;
		if ((new_id->name = strdup(id_name)) == NULL) {
            RaiseError(MEM_ERROR);
        }

        new_id->type    = type;
        new_id->node    = NULL;
        new_id->target_id  = NULL;
        new_id->next_id = actual_scope->first_id;
        actual_scope->first_id = new_id;
	}
	return new_id;
}

/* like a macro, but a function */
static Identifier* Add(identifier_t type, const char* name) {
	return MakeId(type, local_scope, name);
}

static void UnbindId(Identifier* cur_id) {
	debug_printf(MEM_DEBUG, "\ndisconnect: %s%s, %-p", cur_id->name, cur_id->type == FUNCTION ? "()" : "", \
							 cur_id->type == FUNCTION ? NULL : (void*) cur_id->target_id);

	if (cur_id->type == VARIABLE) {
		if (cur_id->target_id) {
            DecreaseUsages(cur_id->target_id);
            cur_id->target_id = NULL;
		}
	} else if (cur_id->type == FUNCTION) {
        cur_id->node = NULL;
    }
}

static void BindId(Identifier* cur_id, void* current_object) {
	debug_printf(MEM_DEBUG, "\nconnect: %s%s, %-p", cur_id->name, cur_id->type == FUNCTION ? "()" : "" , \
							 cur_id->type == FUNCTION ? NULL : (void*) current_object);

	if (cur_id->type == VARIABLE) {
		if (cur_id->target_id)
            UnbindId(cur_id);
        cur_id->target_id = current_object;
	} else if (cur_id->type == FUNCTION)
        cur_id->node = current_object;
}

static void FreeId(Identifier* CurId) {
    UnbindId(CurId);
	free(CurId->name); // IMPORTANT: do not forget to destruct()!!!
	*CurId = NULL_ID;
	free(CurId);
}

static void AddChildScope(bool is_local) {
	Scope* child = NULL;
	if ((child = calloc(1, sizeof(Scope))) == NULL) {
        RaiseError(MEM_ERROR);
    }

	*child = main_scope;
    child->higher_scope = local_scope;
    child->first_id = NULL;
    child->is_nested = is_local;

    local_scope = child;
}


static void FreeChildScope() {
	Identifier* id = NULL;
	Scope* child   = NULL;

    child = local_scope;
	while ((id = child->first_id)) {
        child->first_id = id->next_id; // shifting to the next_list identifier
        FreeId(id);
	}

	if (local_scope != global_scope) {
        local_scope = child->higher_scope;
		free(child);
	} else {
        global_scope->first_id = NULL;
    }
}

#ifdef _LANG_DEBUG_

void FileID_Dump(FILE* stream) {
	int depth         =    0;
	Scope* level      = NULL;
	Identifier* id    = NULL;

	for (level = local_scope, depth = 0; level; level = level->higher_scope, depth++);

	fprintf(stream, "%s - %s - %s - %s \n", "level", "name", "type", "identifier");

	for (level = local_scope; level; level = level->higher_scope, depth--) {
		for (id = level->first_id; id; id = id->next_id) {
			fprintf(stream, "%d - %s - %s;", depth, id->name, IdTypeToString(id->type));
			if (id->target_id != NULL)
				fprintf(stream, "%-p", (void*) id->target_id);
			fprintf(stream, "\n");
		}
	}
}


void ConsoleID_Dump() {
	FILE* id_dump = NULL;
	if ((id_dump = fopen("identifier.dsv", "w")) != NULL) {
        FileID_Dump(id_dump);
		fclose(id_dump);
	}
}
#endif


Identifier identifier = {
        .name        = NULL,
        .next_id     = NULL,
        .target_id   = NULL,

        .add         = Add,
        .search      = search,
        .connect     = BindId,
        .disconnect  = UnbindId,
};


Scope main_scope  = {
        .higher_scope = NULL,
        .first_id     = NULL,

        .add_child    = AddChildScope,
        .remove_child = FreeChildScope,
};
