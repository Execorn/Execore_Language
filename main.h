//
// Created by Legion on 01.12.2021.
//

#ifndef EXECORE_MAIN_H
#define EXECORE_MAIN_H

#include <ctype.h>
#include <stdio.h>

#include "id.h"
#include "processor.h"
#include "src/parser/execore_parser.h"

#include "src/utils/macro_utils.h"

global_configs parser_configs = {NO_DEBUG, TAB_SIZE};


char* my_basename (const char *filename);

void ProcessArgs (const int* argc, char** argv[]);

void ProcessFlags(int* argc, char** argv[]);

void LanguageSupport (char* flag, FILE* stream);

char* my_strrchr (register const char* s, int c);




#endif //EXECORE_MAIN_H
