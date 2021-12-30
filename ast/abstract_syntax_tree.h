//
// Created by Legion on 05.12.2021.
//

#ifndef EXECORE_AST_H
#define EXECORE_AST_H

#include <stdbool.h>
#include "src/library.h"
#include "src/stack.h"
#include "src/array.h"

typedef enum {
    UNARY_NOT = 1,
    UNARY_MINUS,
    UNARY_PLUS,
} unary_t;

typedef enum {
    LITERAL = 1,
    /* operator-type nodes */
    UNARY, BINARY,
    /* code blocks */
    BLOCK, REFERENCE, FUNCTION_CALL,
    /* try to guess lol */
    VARIABLE_DECLARATION, FUNCTION_DECLARATION,
    /* expressions */
    COMMA_EXPR, VARIABLE_INIT, ASSIGNMENT,
    /* statements */
    IF_STATEMENT, PRINT_STATEMENT, RETURN_STATEMENT, EXPRESSION_STATEMENT,
    WHILE_STATEMENT, DO_STATEMENT, PASS_STATEMENT, FOR_STATEMENT,
    IMPORT_STATEMENT, INPUT_STATEMENT, BREAK_STATEMENT, CONTINUE_STATEMENT,

    ARGS, INDEX, SLICE,
} node_t;

/* static is used with inline because of the way gcc handles inline function,
 * GCC performs inline substitution as the part of optimisation, so NodeTypeToString() call may cause error
 */
//TODO: make a normal order (same as in node_t)
static inline char* NodeTypeToString(node_t node_type) {
	static char* names[] = {
		"UNKNOWN", "LITERAL",
        "UNARY", "BINARY", "ARGS",
        "BLOCK", "REFERENCE", "FUNCTION_CALL",
        "ASSIGNMENT",  "VARIABLE_DECLARATION",
		"VARIABLE_INIT", "FUNCTION_DECLARATION", "COMMA_EXPR", "IF_STATEMENT", "PRINT_STATEMENT", "RETURN_STATEMENT", "EXPRESSION_STATEMENT",
		"WHILE_STATEMENT", "DO_STATEMENT", "PASS_STATEMENT", "FOR_STATEMENT", "IMPORT_STATEMENT", "INPUT_STATEMENT",
		"BREAK_STATEMENT", "CONTINUE_STATEMENT", "INDEX", "SLICE",
	};
	return names[node_type >= 0 && node_type < sizeof(names) / sizeof(names[0]) ? node_type : 0];
}

static inline char* UnaryOpToString(unary_t u_operator) {
	static char* names[] = {"UNKNOWN", "NOT", "MINUS", "PLUS"};
	return names[u_operator >= 0 && u_operator < sizeof(names) / sizeof(names[0]) ? u_operator : 0];
}

typedef enum {
    ADD = 1,
    SUB,
    MUL,
    DIV,
    MOD,
    CONJUNCTION,
    DISJUNCTION,
    LESS_OP,
    LESS_EQUAL_OP,
    GREATER_EQUAL_OP,
    GREATER_OP,
    EQUAL_OP,
    NOT_EQUAL_OP,
    IN_OP,
} binary_t;

static inline char* BinaryOpToString(binary_t b_operator) {
	static char* names[] = {
		"UNKNOWN", "ADD", "SUB", "MUL", "DIV", "MOD", "CONJUNCTION",
		"DISJUNCTION", "LESS_OP", "LESS_EQUAL_OP", "GREATER_EQUAL_OP", "GREATER_OP", "EQUAL_OP", "NOT_EQUAL_OP", "IN"
	};

	return names[b_operator > 0 && b_operator < sizeof(names) / sizeof(names[0]) ? b_operator : 0];
}


typedef enum {
    ASSIGN = 1,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
} assign_t;

static inline char* AssignOpToString(assign_t a_operator) {
	static char* names[] = {
		"UNKNOWN", "ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN"
	};
	return names[a_operator > 0 && a_operator < sizeof(names) / sizeof(names[0]) ? a_operator : 0];
}


typedef enum {
    TYPE_CHAR = 1,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_LIST,
} variable_t;

static inline char* VarTypeToString(variable_t var_type) {
	static char* names[] = {"UNKNOWN", "CHAR", "INT", "FLOAT", "STR", "LIST"};

	return names[var_type > 0 && var_type < sizeof(names) / sizeof(names[0]) ? var_type : 0];
}

#include "src/utils/struct_info.h"
typedef struct node {
    NODE_INFO;

    CONTAINERS_UNION;

    NODE_FUNCTIONS;
} Node;

Node* MakeNode(node_t node_type, ...);

#endif
