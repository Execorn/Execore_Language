//
// Created by Legion on 01.12.2021.
//

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "reader.h"
#include "handle_err.h"
#include "processor.h"

static struct {
	int number;
	char* description;
	bool print_extra_info;
} errors[] = {
	{IDENTIFIER_ERROR,         "IDENTIFIER_ERROR",                      true },
	{TYPE_ERROR,               "TYPE_ERROR",                            true },
	{SYNTAX_ERROR,             "SYNTAX_ERROR",                          true },
	{VALUE_ERROR,              "VALUE_ERROR",                           true },
	{SYSTEM_ERROR,             "SYSTEM_ERROR",                          true },
	{INDEX_ERROR,              "INDEX_ERROR: index out of range",       false },
	{MEM_ERROR,                "Out of memory",                         false },
	{OPERATOR_WITH_WRONG_TYPE, "OPERATOR_WITH_WRONG_TYPE",              true },
	{ZERO_DIVISION_ERROR,      "ZERO_DIVISION_ERROR: division by zero", false },
	{LANG_ERROR,               "LANG_ERROR",                            true }
};

static int error_index(const int number) {
	int i = 0;

	while (true) {
		if (errors[i].number == number)
			break;
		if (++i <= (int)(sizeof errors / sizeof errors[0]) - 1)
			continue;
		return -1;
	}

	return i;
}

void RaiseError(const int number, ...) {
	int err_index  =    0;
	char* code_ptr = NULL;
	char* format   = NULL;
	va_list arg_list;

	if ((err_index = error_index(number)) == -1) {
        RaiseError(LANG_ERROR, "lol even err function causes error: wtf is it? %d", number);
    }

	if (current_node || main_reader.source) {
		char* lib_name  = current_node ? current_node->source.lib->name    : main_reader.source->name;
		size_t cur_line = current_node ? current_node->source.current_line : main_reader.source->cur_line_number;
		char* code      = current_node ? current_node->source.lib->buffer  : main_reader.source->buffer;
		size_t buf_idx  = current_node ? current_node->source.char_index   : main_reader.source->buf_line_index;

		fprintf(stderr, "File %s", lib_name);
		fprintf(stderr, ", line %lu\n", (unsigned long) cur_line);

		for (code_ptr = code + buf_idx; *code_ptr && isspace(*code_ptr); code_ptr++);

		for (; *code_ptr && *code_ptr != '\n'; code_ptr++) {
            fprintf(stderr, "%c", *code_ptr);
        }
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "%s", errors[err_index].description);

	va_start(arg_list, number);

	if (errors[err_index].print_extra_info == 1) {
		format = va_arg(arg_list, char*);
		if (format) {
			fprintf(stderr, ": ");
			vfprintf(stderr, format, arg_list);
		}
	}
	fprintf(stderr, "\n");

	va_end(arg_list);


	exit(number);
}
