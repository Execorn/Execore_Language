//
// Created by Legion on 04.12.2021.
//

#ifndef EXECORE_MACRO_UTILS_H
#define EXECORE_MACRO_UTILS_H

#include <stdio.h>

#define MAKE_ERR(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

#define FLAG_FORMAT(argc, argv) (--(*argc) > 0 && (*++(*argv))[0] == '-')

#define  NAME(var)  #var
#define D_PASS (void*)0

#define REPORT(var)                                                                                                 \
    if ((var) == NULL) {                                                                                            \
        fprintf(stderr, "ERROR in function %s: %s is %p. \n", __FUNCTION__, NAME(var), var);                        \
        abort();                                                                                                    \
    }                                                                                                               \
    NULL

#endif //EXECORE_MACRO_UTILS_H
