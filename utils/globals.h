//
// Created by Legion on 02.12.2021.
//

#ifndef EXECORE_CONFIG_H
#define EXECORE_CONFIG_H

#include <stdio.h>

#define _LANG_DEBUG_ 1
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON
// DEBUG IS ON !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define LANG	"ExeCore_"
#define VERSION		"1.8"
#define TAB_SIZE		4


#define BUFFER_SIZE		    128
#define MAX_LINE_LENGTH	    128
#define MAX_NUM_LENGTH	    32
#define TOTAL_IND_MAX    	256


typedef char char_t;
typedef long int_t;
typedef double float_t;

typedef struct {
	int debug_filter;
	int tab_size;
} global_configs;

extern global_configs parser_configs;

#ifdef _LANG_DEBUG_
	#define debug_printf(level, fmt, ...) \
				do { \
					if (parser_configs.debug_filter & (level)) { \
						fprintf(stderr, fmt, __VA_ARGS__); \
					} \
				} while (0)
#else
	#define debug_printf(level, fmt, ...) \
				do { } while (0)
#endif

#define NO_DEBUG               0
#define LEXER_DEBUG            1
#define MEM_DEBUG              2
#define GRAPH_DUMP_EXIT        4
#define GRAPH_DUMP_EXEC        8
#define ID_CONSOLE_DUMP       16
#define ID_FILE_DUMP          32


#ifndef NAEBAL_GCC
#define NAEBAL_GCC(x) (void)(x)
#endif

#endif
