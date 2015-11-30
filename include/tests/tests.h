#ifndef TESTS_H
#define TESTS_H
#include <stdio.h>
#include <string.h>

#define RED "\x1B[0;31m"
#define GREEN "\x1B[0;32m"
#define NOCOLOR "\x1B[0m"

int RETURN_VALUE = 0;

#define PRINTF_FORMAT(x) _Generic(x, int: "%i", long: "%l", \
        unsigned int: "%iu", unsigned long: "%lu", char*: "%s", void*: "%p", \
        struct kheap_metadata*: "%p", default: "Unknown type!")

#define DIAGNOSTICS(a, b) \
        printf("Expected "); \
        printf("%s: ", #a); \
        printf(PRINTF_FORMAT(a), (a)); \
        printf("\n");\
        printf("Actual "); \
        printf("%s: ", #b); \
        printf(PRINTF_FORMAT(b), (b)); \
        printf("\n");


#define EXPECT_EQ(a, b); printf("Test %s == %s ", (#a), (#b));\
if ((a) == (b)) {\
    printf(" %sPassed%s\n", GREEN, NOCOLOR);\
}\
else {\
    printf(" %sFailed%s\n", RED, NOCOLOR);\
    DIAGNOSTICS(a, b); \
    RETURN_VALUE = EXIT_FAILURE;\
}

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

#define EXPECT_NEQ(a, b); printf("Test %s != %s ", (#a), (#b));\
if ((a) != (b)) {\
    printf(" %sPassed%s\n", GREEN, NOCOLOR);\
}\
else {\
    printf(" %sFailed%s\n", RED, NOCOLOR);\
    DIAGNOSTICS(a, b); \
    RETURN_VALUE = EXIT_FAILURE;\
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
