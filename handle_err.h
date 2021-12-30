//
// Created by Legion on 01.12.2021.
//

#ifndef EXECORE_ERR_H
#define EXECORE_ERR_H

#define IDENTIFIER_ERROR 1
#define TYPE_ERROR 2
#define SYNTAX_ERROR 3
#define VALUE_ERROR 4
#define SYSTEM_ERROR 5
#define INDEX_ERROR 6
#define MEM_ERROR 7
#define OPERATOR_WITH_WRONG_TYPE 8
#define ZERO_DIVISION_ERROR 9
#define LANG_ERROR 10

extern void RaiseError(int number, ...);

#endif
