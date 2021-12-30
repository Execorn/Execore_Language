//
// Created by Legion on 08.12.2021.
//

#ifndef EXECORE_VISIT_H
#define EXECORE_VISIT_H

#include "src/ast/abstract_syntax_tree.h"

#ifndef NAEBAL_GCC
#define NAEBAL_GCC(x) (void)(x)
#endif

extern Node* current_node;

extern void AST_byPass(Node* cur_node);
extern void DumpNode(Node* cur_node, int level);
extern void CompileNode(Node* node, Stack* stack);

#endif
