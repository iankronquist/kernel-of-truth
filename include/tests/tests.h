#ifndef TESTS_H
#define TESTS_H
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define RED "\x1B[0;31m"
#define GREEN "\x1B[0;32m"
#define NOCOLOR "\x1B[0m"

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif



int RETURN_VALUE = 0;
int passed;

#define PRINTF_FORMAT(x) _Generic(x, int: "%i", long: "%l", \
        unsigned int: "%u", unsigned long: "%lu", char*: "%s", void*: "%p", \
        bool: "%d", default: "%u")

#define DIAGNOSTICS(a, b) \
        printf("Expected "); \
        printf("%s: ", #b); \
        printf(PRINTF_FORMAT(b), (b)); \
        printf("\n");\
        printf("Actual "); \
        printf("%s: ", #a); \
        printf(PRINTF_FORMAT(a), (a)); \
        printf("\n"); \
        printf("Function %s, file %s, line %d.\n", __FUNCTION__, __FILE__, \
                __LINE__); \

#define _EXPECT_OP(a, b, op); printf("Test %s %s %s ", (#a), (#op), (#b));\
if ((a) op (b)) {\
    printf(" %sPassed%s\n", GREEN, NOCOLOR);\
}\
else {\
    printf(" %sFailed%s\n", RED, NOCOLOR);\
    DIAGNOSTICS(a, b); \
    RETURN_VALUE = EXIT_FAILURE;\
}

#define EXPECT_EQ(a, b) _EXPECT_OP(a, b, ==)
#define EXPECT_NEQ(a, b) _EXPECT_OP(a, b, !=)
#define EXPECT_LEQ(a, b) _EXPECT_OP(a, b, <=)
#define EXPECT_GEQ(a, b) _EXPECT_OP(a, b, >=)
#define EXPECT_GT(a, b) _EXPECT_OP(a, b, >)
#define EXPECT_LT(a, b) _EXPECT_OP(a, b, <)

#define EXPECT_EQ_STR(a, b); printf("Test %s == %s ", (#a), (#b));\
if (strcmp((a), (b)) == 0) {\
    printf(" %sPassed%s\n", GREEN, NOCOLOR);\
}\
else {\
    printf(" %sFailed%s\n", RED, NOCOLOR);\
    DIAGNOSTICS(a, b); \
    RETURN_VALUE = EXIT_FAILURE;\
}

#define EXPECT_NEQ_STR(a, b); printf("Test %s != %s ", (#a), (#b));\
if (strcmp((a), (b)) != 0) {\
    printf(" %sPassed%s\n", GREEN, NOCOLOR);\
}\
else {\
    printf(" %sFailed%s\n", RED, NOCOLOR);\
    DIAGNOSTICS(a, b); \
    RETURN_VALUE = EXIT_FAILURE;\
}

#define EXPECT_SLICE_EMPTY(sequence, begin, end)\
printf("Test %s is empty between %s and %s ", (#sequence), (#begin), (#end));\
passed = 1;\
for (size_t i = (begin); i < (end); i++) {\
    if (sequence[i] != 0) {\
        printf("%sFailed%s\n", RED, NOCOLOR);\
        printf("At index %zu\n", i); \
        DIAGNOSTICS(sequence[i], 0); \
        RETURN_VALUE = EXIT_FAILURE;\
        passed = 0;\
        break;\
    }\
}\
if (passed) {\
    printf("%sPassed%s\n", GREEN, NOCOLOR);\
}

#define EXPECT_CONTAINS(value, sequence, range)\
printf("Test %s in %s ", (#value), (#sequence));\
passed = 0;\
for (size_t i = 0; i < range; i++) {\
    if (sequence[i] == value) {\
        passed = 1;\
        printf("%sPassed%s\n", GREEN, NOCOLOR);\
        break\
    }\
}\
if (!passed) {\
    printf("%sFailed%s\n", RED, NOCOLOR);\
    DIAGNOSTICS(a, b); \
    RETURN_VALUE = EXIT_FAILURE;\
}

#define EXPECT_NCONTAINS(value, sequence, range)\
printf("Test %s in %s ", (#value), (#sequence));\
failed = 0;\
for (size_t i = 0; i < range; i++) {\
    if (sequence[i] == value) {\
        failed = 1;\
        printf("%sFailed%s\n", RED, NOCOLOR);\
        DIAGNOSTICS(a, b); \
        RETURN_VALUE = EXIT_FAILURE;\
    }\
}\
if (!failed) {\
    printf("%sPassed%s\n", GREEN, NOCOLOR);\
}

#endif
