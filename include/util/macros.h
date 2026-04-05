#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__) || defined(__clang__)
    #define Z_UNREACHABLE(STR) do { \
        fprintf(stderr, "REACHED UNREACHABLE: %s (%s:%d)\n", STR, __FILE__, __LINE__); \
        __builtin_unreachable(); \
    } while (0)
#elif defined(_MSC_VER)
    #define Z_UNREACHABLE(STR) do { \
        fprintf(stderr, "REACHED UNREACHABLE: %s (%s:%d)\n", STR, __FILE__, __LINE__); \
        __assume(0); \
    } while (0)
#else
    #define Z_UNREACHABLE(STR) do { \
        fprintf(stderr, "REACHED UNREACHABLE: %s (%s:%d)\n", STR, __FILE__, __LINE__); \
        abort(); \
    } while (0)
#endif

#ifndef ZAPUP_RELEASE
    #define Z_ASSERT(EXPR, MSG) do { \
        if (!(EXPR)) { \
            fprintf(stderr, "ASSERTION FAILED: %s\nMessage: %s\nLocation: %s:%d\n", \
                #EXPR, MSG, __FILE__, __LINE__); \
            abort(); \
        } \
    } while (0)
#else
    #define Z_ASSERT(EXPR, MSG) ((void)0)
#endif
