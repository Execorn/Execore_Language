//
// Created by Legion on 08.12.2021.
//

#include <string.h>

#include "id.h"
#include "function.h"
#include "reader.h"
#include "handle_err.h"
#include "processor.h"
#include "src/types/node_list.h"
#include "src/utils/macro_utils.h"


static int break_flag = 0;
static int cont_flag  = 0;
static int ret_flag   = 0;

Node* current_node = NULL;

static void printf_indent(const int depth, const char* format, ...) {
    REPORT(format);
    
	va_list va_args;
	for (int i = 0; i < depth; i++) {
        printf("| ");
    }

    va_start(va_args, format);
	vprintf(format, va_args);
	va_end(va_args);
}

void DumpNode(Node* cur_node, int depth) {
    REPORT(cur_node);
    
	Node* tmp    = cur_node;
    cur_node     = cur_node;

	printf_indent(depth, "%s\n", NodeTypeToString(cur_node->type));
	cur_node->dump(cur_node, depth);

	if (cur_node->method.is_valid) {
		printf_indent(depth + 1, "METHOD %s\n", cur_node->method.name);

		for (size_t i = 0; i != cur_node->method.args->current_index; i++)
            DumpNode(cur_node->method.args->data[i], depth + 2);
	}

    cur_node = tmp;
}

void AST_byPass(Node* cur_node) {
    REPORT(cur_node);
    
	Node* tmp = current_node;
	current_node = cur_node;

	cur_node->valid_check(cur_node);

	current_node = tmp;
}

void CompileNode(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);
    
	Node* tmp    = current_node;
	current_node =         node;

	node->compile(node, stack);

	if (node->method.is_valid) {
		Target* target = StackPop(stack);
        StackPush(stack, GetTargetMethod(IS_LIST(target) ? LIST_TARGET(target) : target, node->method.name,
                                         node->method.args));
        DecreaseUsages(target);
	}
	current_node = tmp;
}

void Printblock(Node* node, int depth) {
    REPORT(node);
    
	for (size_t i = 0; i != node->block.statements->current_index; i++) {
        DumpNode(node->block.statements->data[i], depth + 1);
    }
}


void ByPassblock(Node* node) {
    REPORT(node);
    
	for (size_t i = 0; i != node->block.statements->current_index; i++) {
        AST_byPass(node->block.statements->data[i]);
    }
}


void Compileblock(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	for (size_t i = 0; i != node->block.statements->current_index && !(break_flag || cont_flag); i++) {
        CompileNode(node->block.statements->data[i], stack);
    }
}


void Printliteral(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "TYPE %s\n", VarTypeToString(node->literal.type));

	switch (node->literal.type) {
		case TYPE_CHAR:
			printf_indent(depth + 1, "VALUE \'%s\'\n", node->literal.value);
			break;

		case TYPE_STRING:
			printf_indent(depth + 1, "VALUE \"%s\"\n", node->literal.value);
			break;

		default:
			printf_indent(depth + 1, "VALUE %s\n", node->literal.value);
            break;
	}
}


void ByPassliteral(Node* node) {
    REPORT(node);

	switch (node->literal.type) {
		case TYPE_CHAR:
            my_stoc(node->literal.value);
			break;

		case TYPE_INT:
            my_stoi(node->literal.value);
			break;

		case TYPE_FLOAT:
            my_stof(node->literal.value);
			break;

		case TYPE_STRING:
			break;

		case TYPE_LIST:
            RaiseError(LANG_ERROR, "idk what that list literal is...");
			break;

		default:
            RaiseError(LANG_ERROR, "wtf is this? %d", node->literal.type);
			break;
	}
}


void Compileliteral(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	switch (node->literal.type) {
		case TYPE_CHAR:
            StackPush(stack, MakeTarget(CHAR_T, my_stoc(node->literal.value)));
			break;

		case TYPE_INT:
            StackPush(stack, MakeTarget(INT_T, my_stoi(node->literal.value)));
			break;

		case TYPE_FLOAT:
            StackPush(stack, MakeTarget(FLOAT_T, my_stof(node->literal.value)));
			break;

		case TYPE_STRING:
            StackPush(stack, MakeTarget(STR_T, node->literal.value));
			break;

		default:
			break;
	}
}


void Printunary(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "OPERATOR %s\n", UnaryOpToString(node->unary.operator));

    DumpNode(node->unary.operand, depth + 1);
}


void ByPassunary(Node* node) {
    REPORT(node);

	switch (node->unary.operator) {
		case UNARY_NOT:
		case UNARY_MINUS:
		case UNARY_PLUS:
			break;

		default:
            RaiseError(LANG_ERROR, "WHAT is that - %d??", node->unary.operator);
			break;
	}

    AST_byPass(node->unary.operand);
}


void Compileunary(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);
    
	Target* target = NULL;

    CompileNode(node->unary.operand, stack);

	switch (node->unary.operator) {
		case UNARY_NOT:
			target = StackPop(stack);
            StackPush(stack, NegateTarget(target));
            DecreaseUsages(target);
			break;

		case UNARY_MINUS:
			target = StackPop(stack);
            StackPush(stack, InvertTarget(target));
            DecreaseUsages(target);
			break;

		case UNARY_PLUS:
			break;
	}
}


void Printbinary(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "OPERATOR %s\n", BinaryOpToString(node->binary.operator));

    DumpNode(node->binary.left, depth + 1);
    DumpNode(node->binary.right, depth + 1);
}


void ByPassbinary(Node* node) {
    REPORT(node);

	switch (node->binary.operator) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case MOD:
		case LESS_OP:
		case LESS_EQUAL_OP:
		case GREATER_OP:
		case GREATER_EQUAL_OP:
		case EQUAL_OP:
		case NOT_EQUAL_OP:
		case IN_OP:
		case CONJUNCTION:
		case DISJUNCTION:
			break;

		default:
            RaiseError(LANG_ERROR, "stop giving me that shit - %d", node->binary.operator);
			break;
	}

    AST_byPass(node->binary.left);
    AST_byPass(node->binary.right);
}


void Compilebinary(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Target* left_operand  = NULL;
    Target* right_operand = NULL;

    CompileNode(node->binary.left, stack);
    left_operand = StackPop(stack);

    CompileNode(node->binary.right, stack);
    right_operand = StackPop(stack);

	switch (node->binary.operator) {
		case ADD:
            StackPush(stack, AddTarget(left_operand, right_operand));
			break;

		case SUB:
            StackPush(stack, SubTarget(left_operand, right_operand));
			break;

		case MUL:
            StackPush(stack, MulTarget(left_operand, right_operand));
			break;

		case DIV:
            StackPush(stack, DivTarget(left_operand, right_operand));
			break;

		case MOD:
            StackPush(stack, ModTarget(left_operand, right_operand));
			break;

		case LESS_OP:
            StackPush(stack, LessTarget(left_operand, right_operand));
			break;

		case LESS_EQUAL_OP:
            StackPush(stack, LessEqualTarget(left_operand, right_operand));
			break;

		case GREATER_OP:
            StackPush(stack, GreaterTarget(left_operand, right_operand));
			break;

		case GREATER_EQUAL_OP:
            StackPush(stack, GreaterEqualTarget(left_operand, right_operand));
			break;

		case EQUAL_OP:
            StackPush(stack, EqualTarget(left_operand, right_operand));
			break;

		case NOT_EQUAL_OP:
            StackPush(stack, NotEqualTarget(left_operand, right_operand));
			break;

		case IN_OP:
            StackPush(stack, LogInTarget(left_operand, right_operand));
			break;

		case CONJUNCTION:
            StackPush(stack, LogAndTarget(left_operand, right_operand));
			break;

		case DISJUNCTION:
            StackPush(stack, LogOrTarget(left_operand, right_operand));
			break;
	}

    DecreaseUsages(left_operand);
    DecreaseUsages(right_operand);
}


void Printcomma_expr(Node* node, int depth) {
    REPORT(node);

	for (size_t i = 0; i != node->commaExpression.expressions->current_index; i++) {
        DumpNode(node->commaExpression.expressions->data[i], depth + 1);
    }
}


void ByPasscomma_expr(Node* node) {
    REPORT(node);

	for (size_t i = 0; i != node->commaExpression.expressions->current_index; i++) {
        AST_byPass(node->commaExpression.expressions->data[i]);
    }
}


void Compilecomma_expr(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	for (size_t cur_expr = 0; cur_expr != node->commaExpression.expressions->current_index; cur_expr++) {
        CompileNode(node->commaExpression.expressions->data[cur_expr], stack);
		if (cur_expr == node->commaExpression.expressions->current_index - 1) break;
		else DecreaseUsages(StackPop(stack));
	}
}


void Printarglist(Node* node, int depth) {
    REPORT(node);

	for (size_t i = 0; i != node->args.arguments->current_index; i++) {
        DumpNode(node->args.arguments->data[i], depth + 1);
    }
}


void ByPassarglist(Node* node) {
    REPORT(node);

	for (size_t i = 0; i != node->args.arguments->current_index; i++) {
        AST_byPass(node->args.arguments->data[i]);
    }
}


void Compilearglist(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Target* target    =  NULL;
    Target* arguments =  NULL;

	target = ConstructTarget(LIST_T);
	for (size_t i = 0; i != node->args.arguments->current_index; i++) {
        CompileNode(node->args.arguments->data[i], stack);
        arguments = StackPop(stack);
		ListT.append((TargetList*) target, CopyTarget(arguments));
        DecreaseUsages(arguments);
	}

    StackPush(stack, target);
}


void Printindex(Node* node, int depth) {
    REPORT(node);

    DumpNode(node->index.seq, depth + 1);
    DumpNode(node->index.index, depth + 1);
}


void ByPassindex(Node* node) {
    REPORT(node);

    AST_byPass(node->index.seq);
    AST_byPass(node->index.index);
}


void Compileindex(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Target* target  = NULL;
    Target* seq     = NULL;
    Target* idx     = NULL;

    CompileNode(node->index.seq, stack);
    seq = StackPop(stack);

    CompileNode(node->index.index, stack);
    idx = StackPop(stack);
	target = GetContentTarget(seq, TargetToInt(idx));

    DecreaseUsages(idx);
    DecreaseUsages(seq);

    StackPush(stack, target);
}


void Printslice(Node* node, int depth) {
    REPORT(node);

    DumpNode(node->slice.seq, depth + 1);
    DumpNode(node->slice.start, depth + 1);
    DumpNode(node->slice.end, depth + 1);
}


void ByPassslice(Node* node) {
    REPORT(node);

    AST_byPass(node->slice.seq);
    AST_byPass(node->slice.start);
    AST_byPass(node->slice.end);
}


void Compileslice(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Target* target = NULL;
    Target* seq    = NULL;
    Target* begin  = NULL;
    Target* end    = NULL;

    CompileNode(node->slice.seq, stack);
    seq = StackPop(stack);

    CompileNode(node->slice.start, stack);
    begin = StackPop(stack);

    CompileNode(node->slice.end, stack);
	end = StackPop(stack);

	target = SliceTarget(seq, TargetToInt(begin), TargetToInt(end));

    DecreaseUsages(end);
    DecreaseUsages(begin);
    DecreaseUsages(seq);

    StackPush(stack, target);
}


void Printassignment(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "OPERATOR %s\n", AssignOpToString(node->assignment.operator));

    DumpNode(node->assignment.variable, depth + 1);
    DumpNode(node->assignment.expression, depth + 1);
}


void ByPassassignment(Node* node) {
    REPORT(node);

	switch (node->assignment.operator) {
		case ASSIGN:
		case ADD_ASSIGN:
		case SUB_ASSIGN:
		case MUL_ASSIGN:
		case DIV_ASSIGN:
		case MOD_ASSIGN:
			break;

		default:
            RaiseError(LANG_ERROR, "wtf is this - %d?", node->assignment.operator);
			break;
	}

    AST_byPass(node->assignment.variable);
    AST_byPass(node->assignment.expression);
}


void Compileassignment(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

    Target* target      = NULL;
    Target* ass_val     = NULL;
    Target* target_copy = NULL;

    CompileNode(node->assignment.variable, stack);
	target = StackPop(stack);

    CompileNode(node->assignment.expression, stack);
    ass_val = StackPop(stack);

	switch (node->assignment.operator) {
		case ASSIGN:
            target_copy = CopyTarget(ass_val);
			break;

		case ADD_ASSIGN:
            target_copy = AddTarget(target, ass_val);
			break;

		case SUB_ASSIGN:
            target_copy = SubTarget(target, ass_val);
			break;

		case MUL_ASSIGN:
            target_copy = MulTarget(target, ass_val);
			break;

		case DIV_ASSIGN:
            target_copy = DivTarget(target, ass_val);
			break;

		case MOD_ASSIGN:
            target_copy = ModTarget(target, ass_val);
			break;

		default:
            target_copy = ConstructTarget(NAN_T);
            break;
	}

    AssignTarget(target, target_copy);
    DecreaseUsages(target_copy);
    DecreaseUsages(ass_val);

    StackPush(stack, target);
}


void Printreference(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "NAME %s\n", node->reference.name);
}


void ByPassreference(Node* node) {
    REPORT(node);

	Identifier* checked = NULL;

	if (!(checked = identifier.search(node->reference.name))) {
        RaiseError(IDENTIFIER_ERROR, "what is %s? huh...", node->reference.name);
    }

    if (checked != NULL) {
        if (checked->type != VARIABLE) {
            RaiseError(TYPE_ERROR, "boss, %s is not variable. idiot!", node->reference.name);
        }
    }
}


void Compilereference(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Identifier* ref = NULL;

    ref = identifier.search(node->reference.name);

    IncreaseUsages(ref->target_id);
    StackPush(stack, ref->target_id);
}


void Printfunction_call(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "NAME %s\n", node->funcCall.name);
	printf_indent(depth + 1, "BUILTIN = %s\n", node->funcCall.is_builtin == true ? "TRUE" : "FALSE");

	for (size_t i = 0; i != node->funcCall.arguments->current_index; i++) {
        DumpNode(node->funcCall.arguments->data[i], depth + 1);
    }
}


void ByPassfunction_call(Node* node) {
    REPORT(node);

	Identifier* func_id = NULL;

	if (node->funcCall.is_builtin == false) {
		if (node->funcCall.is_checked == false) {
			node->funcCall.is_checked = !node->funcCall.is_checked;

			if ((func_id = identifier.search(node->funcCall.name)) == NULL) {
                RaiseError(IDENTIFIER_ERROR, "what the fuck id %s is?", node->funcCall.name);
            }
			if (func_id->node->type != FUNCTION_DECLARATION) {
                RaiseError(TYPE_ERROR, "this id - %s is something not function tho", node->funcCall.name);
            }

			if (func_id->node->functionDecl.arguments->current_index != node->funcCall.arguments->current_index) {
                RaiseError(SYNTAX_ERROR, "I want %d, wtf is %d?",
                           func_id->node->functionDecl.arguments->current_index,
                           node->funcCall.arguments->current_index);
            }


			main_scope.add_child(func_id->node->functionDecl.is_nested); // local variables scope
			for (size_t idx = 0; idx != func_id->node->functionDecl.arguments->current_index; idx++) {
                identifier.add(VARIABLE, func_id->node->functionDecl.arguments->data[idx]);
            }

            AST_byPass(func_id->node->functionDecl.block);
			main_scope.remove_child();
		}
	} else {
		if (node->funcCall.arguments->current_index != GetBuiltinArgc(node->funcCall.name)) {
            RaiseError(SYNTAX_ERROR, "%s wants stack %d, wtf is %d?...",
                       node->funcCall.name, GetBuiltinArgc(node->funcCall.name),
                       node->funcCall.arguments->current_index);
        }
	}
}


void Compilefunction_call(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Node* func          = NULL;
	Identifier* func_id = NULL;
	Array* args         = ArrayConstructor();

	for (size_t arg_idx = 0; arg_idx != node->funcCall.arguments->current_index; arg_idx++) {
        CompileNode(node->funcCall.arguments->data[arg_idx], stack);
        AppendNode(args, StackPop(stack));
	}

	if (node->funcCall.is_builtin) {
        CompileBuiltin(node->funcCall.name, args, stack);
    } else {
		func_id = identifier.search(node->funcCall.name);
        func = func_id->node;

		main_scope.add_child(func_id->node->functionDecl.is_nested);
		for (size_t i = 0; i != args->current_index; i++) {
            func_id = identifier.add(VARIABLE, func->functionDecl.arguments->data[i]);
			identifier.connect(func_id, CopyTarget(args->data[i]));
            DecreaseUsages(args->data[i]);
		}

        CompileNode(func->functionDecl.block, stack);
		main_scope.remove_child();

		if (!ret_flag) {
            StackPush(stack, MakeTarget(INT_T, 0));
            //default return (if 'return' is not at the end of function)
        }
		ret_flag = 0;
	}
    ArrayDestructor(args);
}

void Printexpression_stmnt(Node* node, int depth) {
    REPORT(node);

    DumpNode(node->exprStatement.expression, depth + 1);
}

void ByPassexpression_stmnt(Node* node) {
    REPORT(node);

    AST_byPass(node->exprStatement.expression);
}

void Compileexpression_stmnt(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Target* target = NULL;

    CompileNode(node->exprStatement.expression, stack);

	target = StackPop(stack);
    DecreaseUsages(target);
}

void Printfunction_declaration(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "NAME %s\n", node->functionDecl.name);
	printf_indent(depth + 1, "NESTED = %s\n", node->functionDecl.is_nested == true ? "TRUE" : "FALSE");
	printf_indent(depth + 1, "ARGUMENTS ");

	for (size_t i = 0; i != node->functionDecl.arguments->current_index; i++) {
        if (!i) printf("%s", (char*) node->functionDecl.arguments->data[i]);
        else printf(", %s", (char*) node->functionDecl.arguments->data[i]);
    }
	printf("\n");

    DumpNode(node->functionDecl.block, depth + 1);
}


void ByPassfunction_declaration(Node* node) {
    REPORT(node);

	Identifier* func_id = NULL;
	if (IsBuiltinFunc(node->functionDecl.name)) {
        RaiseError(IDENTIFIER_ERROR, "boss, we already coded %s for you, why do you try to code it again?(",
                   node->functionDecl.name);
    }

	if ((func_id = identifier.add(FUNCTION, node->functionDecl.name)) == NULL) {
        RaiseError(IDENTIFIER_ERROR, "identifier %s already declared", node->functionDecl.name);
    }

	identifier.connect(func_id, node);
	main_scope.add_child(node->functionDecl.is_nested);

	for (size_t i = 0; i != node->functionDecl.arguments->current_index; i++) {
        identifier.add(VARIABLE, (char*) node->functionDecl.arguments->data[i]);
    }
    AST_byPass(node->functionDecl.block);

	main_scope.remove_child();
}


void Compilefunction_declaration(Node* node, Stack* stack) {
    REPORT(node);
    // stack check is unnecessary, its unused anyway

	NAEBAL_GCC(stack);
	Identifier* func_id = NULL;

    func_id = identifier.add(FUNCTION, node->functionDecl.name);
	identifier.connect(func_id, node);
}


void Printvariable_declaration(Node* node, int depth) {
    REPORT(node);
    
	for (size_t i = 0; i != node->VarDecl.defvars->current_index; i++) {
        DumpNode(node->VarDecl.defvars->data[i], depth + 1);
    }
}


void ByPassvariable_declaration(Node* node) {
    REPORT(node);

	for (size_t i = 0; i != node->VarDecl.defvars->current_index; i++) {
        AST_byPass(node->VarDecl.defvars->data[i]);
    }
}


void Compilevariable_declaration(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	for (size_t i = 0; i != node->VarDecl.defvars->current_index; i++)
        CompileNode(node->VarDecl.defvars->data[i], stack);
}


void Printdefvar(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "NAME %s\n", node->newVariable.name);
	printf_indent(depth + 1, "TYPE %s\n", VarTypeToString(node->newVariable.type));

	if (node->newVariable.value) {
        DumpNode(node->newVariable.value, depth + 1);
    }
}


void ByPassdefvar(Node* node) {
    REPORT(node);

	if (IsBuiltinFunc(node->newVariable.name)) {
        RaiseError(IDENTIFIER_ERROR, "dont try to define %s again, it's mine!", node->newVariable.name);
    }

	if (identifier.add(VARIABLE ,node->newVariable.name) == NULL) {
        RaiseError(IDENTIFIER_ERROR, "why the fuck would you declare %s again?", node->newVariable.name);
    }

	if (node->newVariable.value) {
        AST_byPass(node->newVariable.value);
    }
}


void Compiledefvar(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Identifier* defvar_id = NULL;
	Target* target        = NULL;

    defvar_id = identifier.add(VARIABLE, node->newVariable.name);

	switch (node->newVariable.type) {
		case TYPE_CHAR:
			target = ConstructTarget(CHAR_T);
			break;

		case TYPE_INT:
			target = ConstructTarget(INT_T);
			break;

		case TYPE_FLOAT:
			target = ConstructTarget(FLOAT_T);
			break;

		case TYPE_STRING:
			target = ConstructTarget(STR_T);
			break;

		case TYPE_LIST:
			target = ConstructTarget(LIST_T);
			break;

		default:
			target = ConstructTarget(NAN_T);
            break;
	}

	identifier.connect(defvar_id, target);
	if (node->newVariable.value) {
        CompileNode(node->newVariable.value, stack);
		target = StackPop(stack);
        AssignTarget(defvar_id->target_id, target);
        DecreaseUsages(target);
	}
}


void Printif_stmnt(Node* node, int depth) {
    REPORT(node);

    DumpNode(node->ifStatement.condition, depth + 1);
    DumpNode(node->ifStatement.if_true, depth + 1);

	if (node->ifStatement.if_false) {
        DumpNode(node->ifStatement.if_false, depth + 1);
    }
}


void ByPassif_stmnt(Node* node) {
    REPORT(node);

    AST_byPass(node->ifStatement.condition);
    AST_byPass(node->ifStatement.if_true);

	if (node->ifStatement.if_false) {
        AST_byPass(node->ifStatement.if_false);
    }
}


void Compileif_stmnt(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	Target* target = NULL;

    CompileNode(node->ifStatement.condition, stack);
	target = StackPop(stack);

	if (TargetToBool(target)) {
        CompileNode(node->ifStatement.if_true, stack);
    } else if (node->ifStatement.if_false) CompileNode(node->ifStatement.if_false, stack);

    DecreaseUsages(target);
}


void Printwhile_stmnt(Node* node, int depth) {
    REPORT(node);

    DumpNode(node->loopStatement.condition, depth + 1);
    DumpNode(node->loopStatement.block, depth + 1);
}


void ByPasswhile_stmnt(Node* node) {
    REPORT(node);

    AST_byPass(node->loopStatement.condition);
    AST_byPass(node->loopStatement.block);
}


void Compilewhile_stmnt(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	bool condition = false;
	Target* target =  NULL;

    break_flag = cont_flag = 0;
	while (true) {
        CompileNode(node->loopStatement.condition, stack);
		target = StackPop(stack);
		condition = TargetToBool(target);
        DecreaseUsages(target);

		if (!condition || break_flag || ret_flag) break;

        CompileNode(node->loopStatement.block, stack);
        cont_flag = 0;
	}
    break_flag = 0;
}


void Printdo_stmnt(Node* node, int depth) {
    DumpNode(node->loopStatement.block, depth + 1);
    DumpNode(node->loopStatement.condition, depth + 1);
}


void ByPassdo_stmnt(Node* node) {
    AST_byPass(node->loopStatement.block);
    AST_byPass(node->loopStatement.condition);
}


/* do while() implementation */
void Compiledo_stmnt(Node* node, Stack* stack) {
	bool condition;
	Target* target;

    break_flag = cont_flag = 0;

	while (true) {
        CompileNode(node->loopStatement.block, stack);
        cont_flag = 0;

        CompileNode(node->loopStatement.condition, stack);
		target = StackPop(stack);
		condition = TargetToBool(target);
        DecreaseUsages(target);

		if (!condition || break_flag || ret_flag) break;
	}

    break_flag = 0;
}


void Printfor_stmnt(Node* node, int depth) {
	printf_indent(depth + 1, "TARGET %s\n", node->ForStatement.name);

    DumpNode(node->ForStatement.expression, depth + 1);
    DumpNode(node->ForStatement.block, depth + 1);
}


void ByPassfor_stmnt(Node* node) {
	if (!identifier.search(node->ForStatement.name)) {
        identifier.add(VARIABLE, node->ForStatement.name);
    }

    AST_byPass(node->ForStatement.expression);
    AST_byPass(node->ForStatement.block);
}


void Compilefor_stmnt(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	int_t cycle_len     =    0;
	Target* seq         = NULL;
	Identifier* for_id  = NULL;

	if ((for_id = identifier.search(node->ForStatement.name)) == NULL) {
        for_id = identifier.add(VARIABLE, node->ForStatement.name);
    }

	identifier.connect(for_id, ConstructTarget(NAN_T));
    CompileNode(node->ForStatement.expression, stack);

	seq = StackPop(stack);
    cycle_len = GetTargetLength(seq);

    break_flag = cont_flag = 0;

	for (int_t i = 0; i < cycle_len && !break_flag && !ret_flag; i++) {
		identifier.connect(for_id, GetContentTarget(seq, i));
        CompileNode(node->ForStatement.block, stack);
        cont_flag = 0;
	}

    break_flag = 0;

    DecreaseUsages(seq);
}


void Printprint_stmnt(Node* node, int depth) {
    REPORT(node);

	printf_indent(depth + 1, "RAW = %s\n", node->PrintStatement.raw == true ? "TRUE" : "FALSE");
	for (size_t i = 0; i != node->PrintStatement.expressions->current_index; i++) {
        DumpNode(node->PrintStatement.expressions->data[i], depth + 1);
    }
}


void ByPassprint_stmnt(Node* node) {
    REPORT(node);

	for (size_t i = 0; i != node->PrintStatement.expressions->current_index; i++) {
        AST_byPass(node->PrintStatement.expressions->data[i]);
    }
}


void Compileprint_stmnt(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	bool raw_print = true;
	Target* target = NULL;

	for (size_t idx = 0; idx != node->PrintStatement.expressions->current_index; idx++) {
		if (raw_print) raw_print = !raw_print;
		else {
            if (!node->PrintStatement.raw) printf(" ");
        }

        CompileNode(node->PrintStatement.expressions->data[idx], stack);
		target = StackPop(stack);
        DumpTarget(stdout, target);
        DecreaseUsages(target);
	}

	if (node->PrintStatement.raw == false)
		printf("\n");
}


void Printreturn_stmnt(Node* node, int depth) {
	if (node->ReturnStatement.value)
        DumpNode(node->ReturnStatement.value, depth + 1);
}


void ByPassreturn_stmnt(Node* node) {
	if (node->ReturnStatement.value)
        AST_byPass(node->ReturnStatement.value);
}


void Compilereturn_stmnt(Node* node, Stack* stack) {
	if (!node->ReturnStatement.value)
        StackPush(stack, MakeTarget(INT_T, 0));
	else
        CompileNode(node->ReturnStatement.value, stack);

    ret_flag = 1;
}


void Printimport_stmnt(Node* node, int depth) {
	printf_indent(depth + 1, "MODULE %s\n", node->importStatement.name);

    DumpNode(node->importStatement.code, depth + 1);
}


void ByPassimport_stmnt(Node* node) {
    AST_byPass(node->importStatement.code);
}


void Compileimport_stmnt(Node* node, Stack* stack) {
    CompileNode(node->importStatement.code, stack);
}


void Printinput_stmnt(Node* node, int depth) {
    REPORT(node);

	for (size_t i = 0; i != node->inputStatement.prompts->current_index; i++) {
		if (node->inputStatement.prompts->data[i]) {
			printf_indent(depth + 1, "PROMPT %s\n", (char*) node->inputStatement.prompts->data[i]);
		}
		printf_indent(depth + 1, "IDENTIFIER %s\n", (char*) node->inputStatement.identifiers->data[i]);
	}
}


void ByPassinput_stmnt(Node* node) {
    REPORT(node);

	Identifier* input_id = NULL;

	for (size_t i = 0; i != node->inputStatement.identifiers->current_index; i++) {
		if (!(input_id = identifier.search(node->inputStatement.identifiers->data[i]))) {
            RaiseError(IDENTIFIER_ERROR, "idk what is %s LOL", node->inputStatement.identifiers->data[i]);
        }
        if (input_id != NULL) {
            if (input_id->type != VARIABLE) {
                RaiseError(TYPE_ERROR, "why %s is not a variable? FIX IT.", node->inputStatement.identifiers->data[i]);
            }
        }
	}
}


void Compileinput_stmnt(Node* node, Stack* stack) {
    REPORT(node);
    REPORT(stack);

	NAEBAL_GCC(stack);

	Identifier* input_id = NULL;
	Target* target       = NULL;

	for (size_t i = 0; i != node->inputStatement.identifiers->current_index; i++) {
		if (node->inputStatement.prompts->data[i]) {
            printf("%s", (char*) node->inputStatement.prompts->data[i]);
        }

        input_id = identifier.search(node->inputStatement.identifiers->data[i]);
		target = GetTarget(stdin, TYPE(input_id->target_id));
		identifier.connect(input_id, target);
	}
}


void Printpass_stmnt(Node* node, int depth) {
	NAEBAL_GCC(node);
	NAEBAL_GCC(depth);
}


void ByPasspass_stmnt(Node* node) {
	NAEBAL_GCC(node);
}


void Compilepass_stmnt(Node* node, Stack* stack) {
	NAEBAL_GCC(node);
	NAEBAL_GCC(stack);
}


void Printbreak_stmnt(Node* node, int depth) {
	NAEBAL_GCC(node);
	NAEBAL_GCC(depth);
}


void ByPassbreak_stmnt(Node* node) {
	NAEBAL_GCC(node);
}


void Compilebreak_stmnt(Node* node, Stack* stack) {
    NAEBAL_GCC(node);
    NAEBAL_GCC(stack);
    break_flag = true;
}


void Printcontinue_stmnt(Node* node, int depth) {
	NAEBAL_GCC(node);
	NAEBAL_GCC(depth);
}


void ByPasscontinue_stmnt(Node* node) {
    // NO CHECKS!
	NAEBAL_GCC(node);
}


void Compilecontinue_stmnt(Node* node, Stack* stack) {
	NAEBAL_GCC(node);
	NAEBAL_GCC(stack);
    cont_flag = true;
}
