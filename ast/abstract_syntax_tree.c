//
// Created by Legion on 05.12.2021.
//

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "src/reader.h"
#include "src/handle_err.h"
#include "src/array.h"
#include "abstract_syntax_tree.h"


#define ADVANCE_FUNC_GENERATOR(name, lines)                                                                  \
    static void create_##name(Node* n, va_list argp) {                                                       \
        void ByPass##name(Node*);                                                                            \
        void Compile##name(Node*, Stack*);                                                                   \
        void Print##name(Node*, int);                                                                        \
                                                                                                             \
        n->valid_check = ByPass##name;                                                                       \
        n->compile     = Compile##name;                                                                      \
        n->dump        = Print##name;                                                                        \
        lines                                                                                                \
    }

#define SIMPLE_FUNC_GENERATOR(name, lines)                                                                   \
    static void create_##name(Node* n) {                                                                     \
        void ByPass##name(Node*);                                                                            \
        void Compile##name(Node*, Stack*);                                                                   \
        void Print##name(Node*, int);                                                                        \
                                                                                                             \
        n->valid_check = ByPass##name;                                                                       \
        n->compile     = Compile##name;                                                                      \
        n->dump        = Print##name;                                                                        \
        lines                                                                                                \
    }



SIMPLE_FUNC_GENERATOR(block,
                      n->block.statements = ArrayConstructor();)

SIMPLE_FUNC_GENERATOR(comma_expr,
                      n->commaExpression.expressions = ArrayConstructor();)

SIMPLE_FUNC_GENERATOR(arglist,
                      n->args.arguments = ArrayConstructor();)

SIMPLE_FUNC_GENERATOR(variable_declaration,
                      n->VarDecl.defvars = ArrayConstructor();)

SIMPLE_FUNC_GENERATOR(if_stmnt,
                      NULL;)

SIMPLE_FUNC_GENERATOR(while_stmnt,
                      NULL;)

SIMPLE_FUNC_GENERATOR(continue_stmnt,
                      NULL;)

SIMPLE_FUNC_GENERATOR(break_stmnt,
                      NULL;)

SIMPLE_FUNC_GENERATOR(pass_stmnt,
                      NULL;)

SIMPLE_FUNC_GENERATOR(input_stmnt,
                      n->inputStatement.prompts = ArrayConstructor();
                      n->inputStatement.identifiers = ArrayConstructor();)

ADVANCE_FUNC_GENERATOR(reference,
                       n->reference.name = strdup(va_arg(argp, char*));)

ADVANCE_FUNC_GENERATOR(function_call,
                       n->funcCall.name = strdup(va_arg(argp, char*));
                       n->funcCall.arguments = ArrayConstructor();
                       n->funcCall.is_builtin = va_arg(argp, int);
                       n->funcCall.is_checked = false;)

ADVANCE_FUNC_GENERATOR(return_stmnt,
                       n->ReturnStatement.value = va_arg(argp, Node *);)

ADVANCE_FUNC_GENERATOR(literal,
                       n->literal.type = va_arg(argp, variable_t);
                       n->literal.value = strdup(va_arg(argp, char*));)

ADVANCE_FUNC_GENERATOR(unary,
                       n->unary.operator = va_arg(argp, unary_t);
                       n->unary.operand = va_arg(argp, Node*);)

ADVANCE_FUNC_GENERATOR(binary,
                       n->binary.operator = va_arg(argp, binary_t);
                       n->binary.left = va_arg(argp, Node*);
                       n->binary.right = va_arg(argp, Node*);)

ADVANCE_FUNC_GENERATOR(index,
                       n->index.seq = va_arg(argp, Node *);
                       n->index.index = va_arg(argp, Node*);)

ADVANCE_FUNC_GENERATOR(slice,
                       n->slice.seq = va_arg(argp, Node * );
                       n->slice.start = va_arg(argp, Node* );
                       n->slice.end = va_arg(argp, Node* );)

ADVANCE_FUNC_GENERATOR(assignment,
                       n->assignment.operator = va_arg(argp, assign_t);
                       n->assignment.variable = va_arg(argp, Node*);
                       n->assignment.expression = va_arg(argp, Node*);)

ADVANCE_FUNC_GENERATOR(expression_stmnt,
                       n->exprStatement.expression = va_arg(argp, Node * );)

ADVANCE_FUNC_GENERATOR(function_declaration,
                       n->functionDecl.name = strdup(va_arg(argp, char*));
                       n->functionDecl.is_nested = va_arg(argp, int);
                       n->functionDecl.arguments = va_arg(argp, Array*);)

ADVANCE_FUNC_GENERATOR(defvar,
                       n->newVariable.type = va_arg(argp, variable_t);
                       n->newVariable.name = strdup(va_arg(argp, char*));
                       n->newVariable.value = va_arg(argp, Node*);)

ADVANCE_FUNC_GENERATOR(do_stmnt,
                       n->loopStatement.condition = va_arg(argp, Node * );
                       n->loopStatement.block = va_arg(argp, Node *);)

ADVANCE_FUNC_GENERATOR(for_stmnt,
                       n->ForStatement.name = strdup(va_arg(argp, char*));
                       n->ForStatement.expression = va_arg(argp, Node*);)

ADVANCE_FUNC_GENERATOR(print_stmnt,
                       n->PrintStatement.raw = va_arg(argp, int);
                       n->PrintStatement.expressions = ArrayConstructor();)

ADVANCE_FUNC_GENERATOR(import_stmnt,
                       n->importStatement.name = strdup(va_arg(argp, char*));
                       n->importStatement.code = va_arg(argp, Node*);)

Node* MakeNode(node_t node_type, ...) {
	va_list external_args;
	Node* new_node = NULL;

	if ((new_node = calloc(1, sizeof(Node))) == NULL)
        RaiseError(MEM_ERROR);
	else {
        new_node->type                = node_type;
		new_node->source.lib          = main_reader.source;
        new_node->source.current_line = main_reader.source->cur_line_number;
        new_node->source.char_index   = main_reader.source->buf_line_index;
        new_node->method.is_valid     = false; // method is false as default

		va_start(external_args, node_type);
		switch (node_type) {
			case BLOCK:
                create_block(new_node);
				break;

			case LITERAL:
                create_literal(new_node, external_args);
				break;

			case UNARY:
                create_unary(new_node, external_args);
				break;

			case BINARY:
                create_binary(new_node, external_args);
				break;

			case COMMA_EXPR:
                create_comma_expr(new_node);
				break;

			case ARGS:
                create_arglist(new_node);
				break;

			case INDEX:
                create_index(new_node, external_args);
				break;

			case SLICE:
                create_slice(new_node, external_args);
				break;

			case ASSIGNMENT:
                create_assignment(new_node, external_args);
				break;

			case REFERENCE:
				create_reference(new_node, external_args);
				break;

			case FUNCTION_CALL:
				create_function_call(new_node, external_args);
				break;

			case EXPRESSION_STATEMENT:
				create_expression_stmnt(new_node, external_args);
				break;

			case FUNCTION_DECLARATION:
				create_function_declaration(new_node, external_args);
				break;

			case VARIABLE_DECLARATION:
				create_variable_declaration(new_node);
				break;

			case VARIABLE_INIT:
				create_defvar(new_node, external_args);
				break;

			case IF_STATEMENT:
				create_if_stmnt(new_node);
				break;

			case WHILE_STATEMENT:
				create_while_stmnt(new_node);
				break;

			case DO_STATEMENT:
				create_do_stmnt(new_node, external_args);
				break;

			case FOR_STATEMENT:
				create_for_stmnt(new_node, external_args);
				break;

			case PRINT_STATEMENT:
				create_print_stmnt(new_node, external_args);
				break;

			case RETURN_STATEMENT:
				create_return_stmnt(new_node, external_args);
				break;

			case IMPORT_STATEMENT:
				create_import_stmnt(new_node, external_args);
				break;

			case INPUT_STATEMENT:
				create_input_stmnt(new_node);
				break;

			case PASS_STATEMENT:
				create_pass_stmnt(new_node);
				break;

			case BREAK_STATEMENT:
				create_break_stmnt(new_node);
				break;

			case CONTINUE_STATEMENT:
				create_continue_stmnt(new_node);
				break;

            default:
                RaiseError(LANG_ERROR, "Something wrong happened!");
		}

		va_end(external_args);
	}
	return new_node;
}
