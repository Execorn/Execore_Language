//
// Created by Legion on 01.12.2021.
//

#include <string.h>
#include <limits.h>

#include "src/function.h"
#include "src/reader.h"
#include "src/types/target.h"
#include "src/handle_err.h"
#include "execore_parser.h"
#include "src/utils/macro_utils.h"


static bool in_function_declaration = false;

static Node* GetOrExpr();
static Node* GetAssignExpr();
static Node* GetCommaExpr();
static Node* GetStatements();


static int Check(token_t cur_token) {
    if (main_reader.token == cur_token) {
        main_reader.GetToken();
        return 1;
    }
    return 0;
}

static int Require(token_t cur_token) {
    if (Check(cur_token)) return 1;
    RaiseError(SYNTAX_ERROR, "I wanted the %s, what the fuck is that? - %s",
               tokenName(cur_token), tokenName(main_reader.token));
    return 0;
}

/*
 * [int] | [int(begin):int(end)]
 * if there is no idx expr, returns pointer on the next_lib expr part
 */
static Node* GetIndexExpr(Node* root_node) {
    REPORT(root_node);

    char buffer[MAX_NUM_LENGTH] = "";
    Node* seq = NULL;

    if (Check(LEFT_SEQ_BLOCK)) {
        Node* idx   = NULL;
        Node* begin = NULL;
        Node* end   = NULL;
        enum {
            IS_INDEX,
            IS_SLICE,
        } node_type;

        while (true) {
            node_type = IS_INDEX;

            if (Check(COLON)) {
                begin = MakeNode(LITERAL, TYPE_INT, "0");
                node_type = IS_SLICE;
            } else {
                begin = idx = GetOrExpr();
            }

            if (Check(COLON)) node_type = IS_SLICE;

            if (Check(RIGHT_SEQ_BLOCK)) {
                if (node_type == IS_SLICE) {
                    snprintf(buffer, sizeof(buffer), "%ld", (long) INT_MAX);
                    end = MakeNode(LITERAL, TYPE_INT, buffer);
                }
            } else {
                end = GetOrExpr();
                Require(RIGHT_SEQ_BLOCK);
            }

            seq = (node_type == IS_INDEX ? MakeNode(INDEX, root_node, idx) : MakeNode(SLICE, root_node, begin, end));

            if (Check(LEFT_SEQ_BLOCK)) {
                root_node = seq;
            } else break;
        }
    }

    root_node = (seq == NULL ? root_node : seq);

    if (Check(METHOD)) {
        if (main_reader.token == IDENTIFIER) {

            root_node->method.is_valid = true;
            root_node->method.name = strdup(main_reader.string);
            root_node->method.args = ArrayConstructor();

            Require(IDENTIFIER);
            Require(LEFT_PAR);
            while (Check(RIGHT_PAR) == 0) {
                while (true) {
                    AppendNode(root_node->method.args, GetOrExpr());
                    if (main_reader.token == RIGHT_PAR)
                        break;
                    Require(COMMA);
                }
            }
        } else
            RaiseError(SYNTAX_ERROR, "why are you so dumb? method should be here");
    }

    return root_node;
}

static Node* GetElemExpr() {
    char name[BUFFER_SIZE] =   "";
    Node* root_node        = NULL;

    switch (main_reader.token) {
        case CHAR:
            root_node = MakeNode(LITERAL, TYPE_CHAR, main_reader.string);
            Require(CHAR);
            break;

        case INT:
            root_node = MakeNode(LITERAL, TYPE_INT, main_reader.string);
            Require(INT);
            break;

        case FLOAT:
            root_node = MakeNode(LITERAL, TYPE_FLOAT, main_reader.string);
            Require(FLOAT);
            break;

        case STR:
            root_node = MakeNode(LITERAL, TYPE_STRING, main_reader.string);
            Require(STR);
            break;

            /* index contain an expression, for example [a + b / 4] */
        case LEFT_SEQ_BLOCK:
            root_node = MakeNode(ARGS);
            Require(LEFT_SEQ_BLOCK);
            while (!Check(RIGHT_SEQ_BLOCK)) {
                while (true) {
                    AppendNode(root_node->args.arguments, GetAssignExpr());
                    if (main_reader.token == RIGHT_SEQ_BLOCK)
                        break;
                    Require(COMMA);
                }
            }
            break;

        case IDENTIFIER:
            snprintf(name, sizeof(name), "%s", main_reader.string);
            Require(IDENTIFIER);
            if (Check(LEFT_PAR)) {
                root_node = MakeNode(FUNCTION_CALL, name, IsBuiltinFunc(name));
                while (!Check(RIGHT_PAR)) {
                    while (true) {
                        AppendNode(root_node->funcCall.arguments, GetAssignExpr());
                        if (main_reader.token == RIGHT_PAR)
                            break;
                        Require(COMMA);
                    }
                }
            } else
                root_node = MakeNode(REFERENCE, name);
            break;

        case LEFT_PAR: // '('
            Require(LEFT_PAR);
            root_node = GetCommaExpr();
            Require(RIGHT_PAR);
            break;

        default:
            RaiseError(SYNTAX_ERROR, "you dumb ass, expression should be here, nothing else");
            break;
    }

    return GetIndexExpr(root_node);
}

static Node* GetUnaryExpr() {
    Node* value = NULL;

    if (Check(NOT))
        value = MakeNode(UNARY, UNARY_NOT, GetElemExpr());
    else if (Check(MINUS))
        value = MakeNode(UNARY, UNARY_MINUS, GetElemExpr());
    else if (Check(PLUS))
        value = MakeNode(UNARY, UNARY_PLUS, GetElemExpr());
    else
        value = GetElemExpr();

    return value;
}

/*
 * GetUnaryExpr ( ( '*' | '/' | '%' ) GetMulExpr )*
 */
static Node* GetMulExpr() {
    Node* value = NULL;

    value = GetUnaryExpr();

    while (true) {
        if (Check(STAR)) {
            value = MakeNode(BINARY, MUL, value, GetUnaryExpr());
        } else if (Check(SLASH)) {
            value = MakeNode(BINARY, DIV, value, GetUnaryExpr());
        } else if (Check(PERCENT)) {
            value = MakeNode(BINARY, MOD, value, GetUnaryExpr());
        } else break;
    }

    return value;
}

/*
 * GetMulExpr ( ( '+' | '-' ) GetAddExpr )*
 */
static Node* GetAddExpr() {
    Node* value = NULL;

    value = GetMulExpr();

    while (true) {
        if (Check(PLUS)) {
            value = MakeNode(BINARY, ADD, value, GetMulExpr());
        } else if (Check(MINUS)) {
            value = MakeNode(BINARY, SUB, value, GetMulExpr());
        } else break;
    }

    return value;
}

/*
 * GetAddExpr ( ( '<'| '>' | '<=' | '>=' ) GetRelateExpr )*
 */
static Node* GetRelateExpr() {
    Node* value = NULL;

    value = GetAddExpr();

    while (true) {
        if (Check(LESS)) {
            value = MakeNode(BINARY, LESS_OP, value, GetRelateExpr());
        } else if (Check(LESS_EQUAL)) {
            value = MakeNode(BINARY, LESS_EQUAL_OP, value, GetRelateExpr());
        } else if (Check(GREATER)) {
            value = MakeNode(BINARY, GREATER_OP, value, GetRelateExpr());
        } else if (Check(GREATER_EQUAL)) {
            value = MakeNode(BINARY, GREATER_EQUAL_OP, value, GetRelateExpr());
        } else break;
    }

    return value;
}

/*
 * GetRelateExpr ( ( '==' | '!=' | '<>' | 'in' ) GetEqualExpr )*
 */
static Node* GetEqualExpr() {
    Node* value = NULL;

    value = GetRelateExpr();

    while (true) {
        if (Check(EQUAL_EQUAL)) {
            value = MakeNode(BINARY, EQUAL_OP, value, GetRelateExpr());
        } else if (Check(NOT_EQUAL)) {
            value = MakeNode(BINARY, NOT_EQUAL_OP, value, GetRelateExpr());
        } else if (Check(IN)) {
            value = MakeNode(BINARY, IN_OP, value, GetRelateExpr());
        } else break;
    }

    return value;
}


/*
 * GetEqualExpr ( 'and' GetAndExpr )*
 */
static Node* GetAndExpr() {
    Node* value = NULL;

    value = GetEqualExpr();

    while (true) {
        if (Check(AND)) {
            value = MakeNode(BINARY, CONJUNCTION, value, GetAndExpr());
        } else break;
    }

    return value;
}

/*
 * GetAndExpr ( 'or' GetOrExpr )*
 */
static Node* GetOrExpr() {
    Node* value = NULL;

    value = GetAndExpr();

    while (true) {
        if (Check(OR)) {
            value = MakeNode(BINARY, DISJUNCTION, value, GetOrExpr());
        } else break;
    }

    return value;
}

/*
 * GetOrExpr ( ( '=' | '+=' | '-=' | '*=' | '\=' | '%=' ) GetAssignExpr )*
 */
static Node* GetAssignExpr() {
    Node* value = NULL;

    value = GetOrExpr();

    while (true) {
        if (Check(EQUAL)) {
            value = MakeNode(ASSIGNMENT, ASSIGN, value, GetAssignExpr());
        } else if (Check(PLUS_EQUAL)) {
            value = MakeNode(ASSIGNMENT, ADD_ASSIGN, value, GetOrExpr());
        } else if (Check(MINUS_EQUAL)) {
            value = MakeNode(ASSIGNMENT, SUB_ASSIGN, value, GetOrExpr());
        } else if (Check(STAR_EQUAL)) {
            value = MakeNode(ASSIGNMENT, MUL_ASSIGN, value, GetOrExpr());
        } else if (Check(SLASH_EQUAL)) {
            value = MakeNode(ASSIGNMENT, DIV_ASSIGN, value, GetOrExpr());
        } else if (Check(PERCENT_EQUAL)) {
            value = MakeNode(ASSIGNMENT, MOD_ASSIGN, value, GetOrExpr());
        } else break;
    }

    return value;
}

/*
 * GetAssignExpr ( ',' GetAssignExpr )+
 */
static Node* GetCommaExpr() {
    Node* comma = NULL;
    Node* expr  = NULL;

    expr = GetAssignExpr();

    if (main_reader.token == COMMA) {
        comma = MakeNode(COMMA_EXPR);
        AppendNode(comma->commaExpression.expressions, expr);

        while (true) {
            if (Check(COMMA)) {
                AppendNode(comma->commaExpression.expressions, GetAssignExpr());
            } else break;
        }
    }
    if (comma) return comma;
    else return expr;
}

/*
 * GetCommaExpr() newline
 */
static Node* GetExprStatement() {
    Node* statement = NULL;
    statement = MakeNode(EXPRESSION_STATEMENT, GetCommaExpr());

    Require(NEWLINE); // newline is required after GetStatement!!
    return statement;
}

/*
 * newline indent GetStatement+ dedent
 */
static Node* GetIndentStatements() {
    Node* indent = NULL;

    Require(NEWLINE);
    Require(INDENT);

    indent = GetStatements();
    Require(DEDENT);

    return indent;
}

/*
 * 'def' identifier '(' (identifier ( ',' identifier )* )? ')' GetStatements
 */
static Node* GetFunctionDecl() {
    bool temp              =    0;
    Node* statement        = NULL;
    char name[BUFFER_SIZE] =   "";

    Array* arguments = ArrayConstructor();

    snprintf(name, sizeof(name), "%s", main_reader.string);

    Require(IDENTIFIER);
    Require(LEFT_PAR);

    while (!Check(RIGHT_PAR)) {
        while (true) {
            if (main_reader.token != IDENTIFIER) {
                RaiseError(SYNTAX_ERROR, "bitch, i want the identifier, what is that shit?- %s",
                           tokenName(main_reader.token));
            }
            AppendNode(arguments, strdup(main_reader.string));

            Require(IDENTIFIER);
            if (main_reader.token == RIGHT_PAR) break;
            Require(COMMA);
        }
    }

    statement = MakeNode(FUNCTION_DECLARATION, name, in_function_declaration, arguments);

    temp = in_function_declaration;
    in_function_declaration = true;

    statement->functionDecl.block = GetIndentStatements();

    in_function_declaration = temp;

    return statement;
}

/*
 * type identifier ( '=' GetAssignExpr )? ( ',' identifier ( '=' GetAssignExpr )? )* newline
 */
static Node* GetVariableDecl(variable_t type){
    char name[BUFFER_SIZE] =   "";
    Node* root_node        = NULL;
    Node* variable         = NULL;

    root_node = MakeNode(VARIABLE_DECLARATION);

    while (true) {
        if (main_reader.token != IDENTIFIER)
            RaiseError(SYNTAX_ERROR, "bitch, i want the identifier, what is that shit?- %s", tokenName(main_reader.token));

        snprintf(name, sizeof(name), "%s", main_reader.string);
        main_reader.GetToken();

        variable = MakeNode(VARIABLE_INIT, type, name, Check(EQUAL) ? GetAssignExpr() : NULL);
        AppendNode(root_node->VarDecl.defvars, variable);

        if (Check(NEWLINE)) break;
        Require(COMMA);
    }

    return root_node;
}

/*
 * 'if' statement ( 'else' statement )?
 */
static Node* GetIfStatement() {
    Node* true_condition   = NULL;
    Node* false_condition  = NULL;

    true_condition = MakeNode(IF_STATEMENT);

    true_condition->ifStatement.condition   = GetCommaExpr();

    true_condition->ifStatement.if_true     = GetIndentStatements();

    if (Check(ELSE)) false_condition        = GetIndentStatements();

    true_condition->ifStatement.if_false    = false_condition;

    return true_condition;
}

/*
 * 'while' GetCommaExpr GetIndentStatements
 */
static Node* GetWhileStatement() {
    Node* statement = NULL;

    statement = MakeNode(WHILE_STATEMENT);

    statement->loopStatement.condition = GetCommaExpr();
    statement->loopStatement.block     = GetIndentStatements();

    return statement;
}

/*
 *'do' GetStatements 'while' expr newline
 */
static Node* GetDoStatement() {
    Node* statement  = NULL;
    Node* condition  = NULL;
    Node* next_cond  = NULL;

    next_cond = GetIndentStatements();

    Require(WHILE);

    condition = GetCommaExpr();

    statement = MakeNode(DO_STATEMENT, condition, next_cond);

    Require(NEWLINE);

    return statement;
}

/*
 * 'for' ident 'in' seq newline GetStatements
 */
static Node* GetForStatement() {
    char var_name[BUFFER_SIZE] =   "";
    Node* statement            = NULL;

    if (main_reader.token == IDENTIFIER) snprintf(var_name, sizeof(var_name), "%s", main_reader.string);

    Require(IDENTIFIER);
    Require(IN);

    statement = MakeNode(FOR_STATEMENT, var_name, GetCommaExpr());

    if (main_reader.token != NEWLINE) RaiseError(SYNTAX_ERROR, "where the fuck is newline?");

    statement->ForStatement.block = GetIndentStatements();

    return statement;
}

/*
 *'print' '-raw'? ( assignment_expr ( ',' GetAssignExpr )* )? newline
 */
static Node* GetPrintStatement() {
    Node* root_node  =  NULL;
    bool is_raw      = false;

    if (main_reader.token == MINUS) {
        if (main_reader.PeekToken() == IDENTIFIER &&
            !strcmp(main_reader.string, "raw")) {
            main_reader.GetToken();
            main_reader.GetToken();
            is_raw = true;
        }
    }

    root_node = MakeNode(PRINT_STATEMENT, is_raw);

    while (!Check(NEWLINE)) {
        while (true) {
            AppendNode(root_node->PrintStatement.expressions, GetAssignExpr());
            if (main_reader.token == NEWLINE) {
                break;
            }
            Require(COMMA);
        }
    }

    return root_node;
}

/*
 * 'return' expression? newline
 */
static Node* GetReturnStatement() {
    Node* statement      = NULL;
    Node* returned_value = NULL;

    if (main_reader.token != NEWLINE) returned_value = GetCommaExpr();

    statement = MakeNode(RETURN_STATEMENT, returned_value);
    Require(NEWLINE); // new line is required after return

    return statement;
}


static Node* GetInputStatement() {
    Node* root_node = NULL;

    root_node = MakeNode(INPUT_STATEMENT);

    /* do {} while() used because even if there is no comma, one input statement should be gotten */
    do {
        if (main_reader.token == STR) {
            AppendNode(root_node->inputStatement.prompts, strdup(main_reader.string));
            main_reader.GetToken();
        } else
            AppendNode(root_node->inputStatement.prompts, NULL);
        if (main_reader.token == IDENTIFIER)
            AppendNode(root_node->inputStatement.identifiers, strdup(main_reader.string));
        Check(IDENTIFIER); // to skip identifier if it is it, else nothing happens
    } while (Check(COMMA));

    Require(NEWLINE);

    return root_node;
}

/*
 * 'import' string newline
 */
static Node* GetImportStatement() {
    Node* lib_buffer = NULL;
    Node* statement  = NULL;

    if (current_library.FindLibrary(main_reader.string) != NULL) {
        RaiseError(SYNTAX_ERROR, "why the fuck would you import %s library again?", main_reader.string);
    }

    lib_buffer = ParseLibrary(current_library.GetLibrary(main_reader.string));

    statement = MakeNode(IMPORT_STATEMENT, main_reader.string, lib_buffer);

    Require(STR);
    Require(NEWLINE);

    return statement;
}


/*
 * 'pass' newline
 */
static Node* GetPassStatement() {
    Node* statement =               NULL;
    statement       = MakeNode(PASS_STATEMENT);

    Require(NEWLINE);

    return statement;
}


/*
 * 'break' newline
 */
static Node* GetBreakStatement() {
    Node* statement = MakeNode(BREAK_STATEMENT);

    Require(NEWLINE);

    return statement;
}


/*
 * 'continue' newline
 */
static Node* GetContinueStatement() {
    Node* statement = MakeNode(CONTINUE_STATEMENT);

    Require(NEWLINE);

    return statement;
}


static Node* GetStatement() {
    Node* statement = NULL;

    if (Check(CHAR_INIT))
        statement = GetVariableDecl(TYPE_CHAR);
    else if (Check(INT_INIT))
        statement = GetVariableDecl(TYPE_INT);
    else if (Check(FLOAT_INIT))
        statement = GetVariableDecl(TYPE_FLOAT);
    else if (Check(STR_INIT))
        statement = GetVariableDecl(TYPE_STRING);
    else if (Check(LIST_INIT))
        statement = GetVariableDecl(TYPE_LIST);
    else if (Check(FUNC_INIT))
        statement = GetFunctionDecl();
    else if (Check(IF))
        statement = GetIfStatement();
    else if (Check(WHILE))
        statement = GetWhileStatement();
    else if (Check(DO))
        statement = GetDoStatement();
    else if (Check(PRINT))
        statement = GetPrintStatement();
    else if (Check(RETURN))
        statement = GetReturnStatement();
    else if (Check(PASS))
        statement = GetPassStatement();
    else if (Check(FOR))
        statement = GetForStatement();
    else if (Check(BREAK))
        statement = GetBreakStatement();
    else if (Check(CONTINUE))
        statement = GetContinueStatement();
    else if (Check(IMPORT))
        statement = GetImportStatement();
    else if (Check(INPUT))
        statement = GetInputStatement();
    else if (Check(END_TRIGGER))
        statement = NULL;
    else
        statement = GetExprStatement();

    return statement;
}


/*
 * newline indent statement+ dedent
 */
static Node* GetStatements() {
    Node* cur_block = NULL;
    Node* statement = NULL;

    cur_block = MakeNode(BLOCK);

    while (true) {
        if ((statement = GetStatement())) {
            AppendNode(cur_block->block.statements, statement);
        }
        if (main_reader.token == DEDENT || main_reader.token == END_TRIGGER) break;
    }

    return cur_block;
}


Node* ParseLibrary(library* w_lib) {
    REPORT(w_lib);

    Node* root_node = NULL;

    reader lib_scanner;

    main_reader.CopyReader(&lib_scanner);
    main_reader.MakeReader(&main_reader, w_lib); // setting default values

    main_reader.GetToken();

    root_node = GetStatements();

    main_reader.UploadReader(&lib_scanner);

    return root_node; // output is constructed sub-tree USING STACK
}
