//
// Created by Legion on 01.12.2021.
//

#include "main.h"

int	main(int argc, char* argv[]) {
    ProcessFlags(&argc, &argv);

    ProcessArgs( &argc, &argv);

    return 0;
}

void ProcessArgs(const int* argc, char** argv[]) {
    char* source = my_basename(**argv);

    switch (*argc) {
        case 0: {
            MAKE_ERR("%s: where the fuck is the source file?...\n", source);
            LanguageSupport(source, stderr);
            break;
        }

        case 1: {
            Stack* ext_stack = StackConstructor(10);

            int code = 0;
            Node* root_node = ParseLibrary(current_library.GetLibrary(**argv));


            if (parser_configs.debug_filter & (GRAPH_DUMP_EXEC | GRAPH_DUMP_EXIT))
                DumpNode(root_node, 0);

            global_configs gbs;
            gbs.debug_filter = parser_configs.debug_filter;
            parser_configs.debug_filter = NO_DEBUG;

            AST_byPass(root_node);
            main_scope.remove_child();

            parser_configs.debug_filter = gbs.debug_filter;

            /* begin processing */
            CompileNode(root_node, ext_stack);

            if (!IsEmptyStack(ext_stack)) {
                Target* new_target = StackPop(ext_stack);
                if (IS_NUM(new_target)) {
                    code = (int) TargetToInt(new_target);
                }
                DecreaseUsages(new_target);
            }

#ifdef _LANG_DEBUG_
            if (parser_configs.debug_filter & (ID_CONSOLE_DUMP | ID_FILE_DUMP)) {
                printf("\nin stack rn: %ld. \n", ext_stack->top + 1);

                while (!IsEmptyStack(ext_stack)) {
                    DumpTarget(stderr, StackPop(ext_stack));
                }
            }

            if (parser_configs.debug_filter & ID_FILE_DUMP) {
                TargetFile_dump(stderr);
            }

            if (parser_configs.debug_filter & ID_FILE_DUMP) {
                ConsoleID_Dump();
                ConsoleTarget_Dump();
            }
#endif  /* _LANG_DEBUG_ */
            exit(code);
        }

        default:
            LanguageSupport(source, stderr);
            exit(0);
            break;
    }
}

void ProcessFlags(int* argc, char** argv[]) {
    char* cmd = my_basename(**argv);

    while (FLAG_FORMAT(argc, argv)) {
        char current_flag = *(++(**argv)); // pointing to the next_lib string's first symbol

        switch (current_flag) {
            case 'h':
                LanguageSupport(cmd, stdout);
                exit(0);
                break;

            case 't':
                if (isdigit(*(++(**argv)))) {
                    parser_configs.tab_size = (int) my_stoi(**argv);
                    if (parser_configs.tab_size < 1) {
                        fprintf(stderr, "%s: invalid tab_size %d\n", \
										cmd, parser_configs.tab_size);
                        parser_configs.tab_size = TAB_SIZE;
                    }
                } else
                    parser_configs.tab_size = TAB_SIZE;
                break;

            case 'v':
                fprintf(stdout, "%s version %s\n", LANG, VERSION);
                exit(0);
                break;

#ifdef _LANG_DEBUG_
            case 'D':
                if (isdigit(*((**argv) += 2))) {
                    /* setting debug_filter for parser and compiler */
                    parser_configs.debug_filter = (int) my_stoi(**argv);
                } else {
                    /* default debug_filter */
                    parser_configs.debug_filter = GRAPH_DUMP_EXEC;
                }
                break;
#endif  /* _LANG_DEBUG_ */

            default:
                fprintf(stderr, "%s: unknown flag - %c, use -h to get help info \n", cmd, current_flag);
                LanguageSupport(cmd, stderr);
                exit(0);
                break;
        }
    }
}

void LanguageSupport(char* flag, FILE* stream) {
    fprintf(stream, "[!] %s LANG SUPPORT [!] \n", LANG);
    fprintf(stream, "[!]        INFORMATION        [!] \n\n"         );
    fprintf(stream, "Version: %s. \n",                       VERSION);
    fprintf(stream, "Command: %s [options] [source file] \n",   flag);

    fprintf(stream, "[!]          OPTIONS         [!]\n"            );


#ifdef _LANG_DEBUG_
    fprintf(stream, "-D[debug_code] = show debug information \n");
    fprintf(stream, "    default debug = %d \n", GRAPH_DUMP_EXEC);
    fprintf(stream, "    debug_code cases: \n");
    fprintf(stream, "    %2d: disable debugger \n", NO_DEBUG);
    fprintf(stream, "    %2d: lexemes debug \n", LEXER_DEBUG);
    fprintf(stream, "    %2d: memory debug \n", MEM_DEBUG);
    fprintf(stream, "    %2d: make graphic dump and abort \n", GRAPH_DUMP_EXIT);
    fprintf(stream, "    %2d: make graphic dump and continue \n", GRAPH_DUMP_EXEC);
    fprintf(stream, "    %2d: variables console debug \n", ID_CONSOLE_DUMP);
    fprintf(stream, "    %2d: variables file debug \n\n", ID_FILE_DUMP);
#endif  /* _LANG_DEBUG_ */


    fprintf(stream, "-h = help info \n");
    fprintf(stream, "-t[tab_size(default = %d)] = modify language tab bytes \n", TAB_SIZE);
    fprintf(stream, "-v = current version info \n");
}

char* my_basename(const char* filename) {
    char *p = my_strrchr(filename, '/');
    return p ? p + 1 : (char*) filename; // (char*) is here to prevent 'discard qualifiers' warning
}

char* my_strrchr (s, c)
        register const char *s;
        int c; {
    char* return_value = 0;

    do {
        if (*s == c) return_value = (char*) s; // (char*) is here to prevent 'discard qualifiers' warning
    } while (*s++);
    return (return_value);
}