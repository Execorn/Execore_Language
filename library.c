//
// Created by Legion on 02.12.2021.
//

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "library.h"
#include "handle_err.h"
#include "src/utils/macro_utils.h"


static library* loaded_libraries = NULL;

static library* FindLibrary(const char* name) {
    REPORT(name);
    if (*name == '\0') {
        MAKE_ERR("current lib name (%s) is \"%s\" \n", NAME(name), name);
    }

	library* cur_lib = NULL;

	for (cur_lib = loaded_libraries; cur_lib != NULL; cur_lib = cur_lib->next_lib) {
        if (!strcmp(name, cur_lib->name)) break;
    }

	return cur_lib;
}

static int UploadLibrary(library* w_lib, const char* name) {
	FILE* lib_file = NULL;
	struct stat stat_buffer;

	if (!stat(name, &stat_buffer)) {
        w_lib->bytes = stat_buffer.st_size;
		if ((w_lib->buffer = calloc(w_lib->bytes + 2, sizeof(char))) != NULL) {
			if ((lib_file = fopen(name, "r")) != NULL) {
                w_lib->bytes = fread(w_lib->buffer, sizeof(char), w_lib->bytes, lib_file);
                w_lib->buffer[w_lib->bytes++] = '\n';
                w_lib->buffer[w_lib->bytes]   = 0;
				fclose(lib_file);
				return 1;
			} else {
				free(w_lib->buffer);
                w_lib->buffer = NULL;
			}
		}
	}
	return 0;
}


static library* GetLibrary(const char* name) {
    REPORT(name);
    if (*name == '\0') {
        MAKE_ERR("current lib name (%s) is \"%s\" \n", NAME(name), name);
    }

	library* cur_lib = NULL;

	if ((cur_lib = calloc(1, sizeof(library))) == NULL) {
        RaiseError(MEM_ERROR);
    }

	*cur_lib = current_library;

	if (UploadLibrary(cur_lib, name) == 0)
        RaiseError(SYSTEM_ERROR, "error importing %s: %s (%d)", name, strerror(errno), errno);

	if ((cur_lib->name = strdup(name)) == NULL) {
        RaiseError(MEM_ERROR);
    }

    cur_lib->next_lib = loaded_libraries;
    loaded_libraries  = cur_lib;

	return cur_lib;
}

static int GetChar(library* w_lib) {
    REPORT(w_lib);

	if (w_lib->buf_char_index >= w_lib->bytes)
		return EOF;
	else {
		if (w_lib->buf_char_index > 0 && w_lib->buffer[w_lib->buf_char_index - 1] == '\n') {
            w_lib->buf_line_index = w_lib->buf_char_index;
			w_lib->cur_line_number++;
		}
		return w_lib->buffer[w_lib->buf_char_index++];
	}
}

static int PeekChar(library* w_lib) {
    REPORT(w_lib);

	if (w_lib->buf_char_index >= w_lib->bytes) {
        return EOF;
    } else {
        return w_lib->buffer[w_lib->buf_char_index];
    }
}

static int PushChar(library* w_lib, const char symbol) {
    REPORT(w_lib);

	if (w_lib->buf_char_index > 0) {
        w_lib->buf_char_index -= 1;
		if (w_lib->buf_char_index > 0 && w_lib->buffer[w_lib->buf_char_index - 1] == '\n') {
            w_lib->cur_line_number--;
        }
	}

	if (w_lib->buffer[w_lib->buf_char_index] != symbol) {
        MAKE_ERR("PushChar failed: '%c' != '%c' \n)", w_lib->buffer[w_lib->buf_char_index], symbol);
    }

	return symbol;
}


library current_library = {
	.next_lib        = NULL,
	.name            = "",
	.buffer          = "\n",
	.bytes           = 0,
	.buf_char_index  = 0,
	.buf_line_index  = 0,
	.cur_line_number = 1, // lib always have at least 1 line - even if it's empty

	.GetChar         = GetChar,
	.PeekChar        = PeekChar,
	.PushChar        = PushChar,

	.GetLibrary      = GetLibrary,
	.FindLibrary     = FindLibrary,
	};
