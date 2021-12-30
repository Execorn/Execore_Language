//
// Created by Legion on 04.12.2021.
//

#ifndef EXECORE_READER_H
#define EXECORE_READER_H

#include <stdbool.h>
#include "src/utils/globals.h"
#include "library.h"

typedef enum {
    UNKNOWN = 0,
    /* is_builtin types */
    CHAR, INT, FLOAT, STR,
    /* simple operators */
    STAR, SLASH, PLUS, MINUS, PERCENT, EQUAL,
    /* relational operators */
    EQUAL_EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    /* complicated operators */
    PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL,
    /* additional operators */
    COMMA, LEFT_PAR, RIGHT_PAR, LEFT_SEQ_BLOCK, RIGHT_SEQ_BLOCK,
    /* code separators */
    NEWLINE, INDENT, DEDENT, COLON,
    /* keywords */
    ELSE, DO, IF, WHILE, INPUT, PRINT, FOR, IN, NOT,
    RETURN, PASS, BREAK, CONTINUE, IMPORT, AND, OR,
    /* the names speak for themselves */
    IDENTIFIER, METHOD, END_TRIGGER,
    /* defined values for is_builtin types and functions */
    CHAR_INIT, INT_INIT, FLOAT_INIT, STR_INIT, FUNC_INIT, LIST_INIT
} token_t;

//TODO: fix string array order (to the token_t order)
static inline char* tokenName(token_t t) {
	static char* string[] = {
	"UNKNOWN", 
    "CHARACTER LITERAL", "INTEGER LITERAL", "FLOAT LITERAL",
	"STRING LITERAL", 
    "STAR", "DIV", "PLUS", "MINUS", "EQUAL_EQUAL", "NOT_EQUAL",
	"LESS", "LESS_EQUAL", "GREATER", "GREATER_EQUAL", "COMMA", "RIGHT_PAR", "ELSE",
	"DO", "LEFT_PAR", "EQUAL", "NUMBER", "IDENTIFIER", "IF", "WHILE", "INPUT",
	"PRINT", "CHAR_INIT", "INT_INIT", "FLOAT_INIT", "STR_INIT", "FUNC_INIT", "METHOD",
	"END_TRIGGER", "RETURN", "PERCENT", "AND", "OR", "PLUS_EQUAL", "MINUS_EQUAL",
	"STAR_EQUAL", "SLASH_EQUAL", "PERCENT_EQUAL", "NOT", "LEFT_SEQ_BLOCK", "RIGHT_SEQ_BLOCK",
	"NEWLINE", "INDENT", "DEDENT", "PASS", "BREAK", "CONTINUE", "LIST_INIT",
	"COLON", "IMPORT", "FOR", "IN" };

	return string[t > 0 && t < sizeof(string) / sizeof(string[0]) ? t : 0];
}

#include "src/utils/struct_info.h"
typedef struct reader {
	READER_INFO;

	INDENTATION_INFO;

	TOKEN_FUNC;

	READERS_FUNC;
} reader;

extern reader main_reader;

#endif
