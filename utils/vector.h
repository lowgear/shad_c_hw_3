#pragma once

#include <stdlib.h>
#include <string.h>

#include "smartptr_tools.h"

// useful to set it greater for debug
#define DEFSIZE 2

#define DEF_ARRAY(name, T) \
struct name { \
    REFCNT_DEF; \
    size_t size; \
    T array[DEFSIZE]; \
}; \

#define INIT_ARR(v, sz, fallbackAction) do { \
    (v) = (__typeof(v)) malloc((sizeof(*v) + sizeof((v)->array[0]) * (sz)) - sizeof((v)->array[0]) * (DEFSIZE)); \
    if ((v) == NULL) \
        do { fallbackAction; } while (0); \
    (v)->size = (sz); \
    REFCNT(v) = 1; \
} while(0)

#define FREE_A(v, freer) do { \
    IFFREE(v) { \
        for (size_t aiter = 0; aiter < SIZE(v); ++aiter) \
            freer(&ID(v, aiter)); \
        free(v); \
    } \
    v = NULL; \
} while (0)

#define DEF_VECTOR(name, T) \
    struct name { \
        size_t size; \
        size_t cnt; \
        T array[DEFSIZE]; \
    }; \
    \
    typedef struct name* name##_Ptr;

#define ARR(vec) ((vec)->array)
#define ID(vec, id) (ARR(vec)[(id)])
#define CNT(vec) ((vec)->cnt)
#define SIZE(vec) ((vec)->size)

#define ID_P(vec, id) ID(*vec, id)
#define CNT_P(vec) CNT(*vec)
#define SIZE_P(vec) SIZE(*vec)
#define STRUCT_SIZE(vec) (sizeof(vec) + ((vec).size - 1) * sizeof((vec).array[0]))

#define INIT_VEC(vec, s, fallbackAction) \
    do { \
        (vec) = (__typeof(vec)) malloc((sizeof((*(vec))) + (s) * sizeof(ID_P(&vec, 0))) - (DEFSIZE) * sizeof(ID_P(&vec, 0))); \
        if ((vec) == NULL) \
            do { fallbackAction; } while (0); \
        (vec)->size = s; \
        (vec)->cnt = 0; \
    } while (0);

#define PUSH_BACK_P(vecP, val, fallbackAction) \
    do { \
        if (CNT_P(vecP) == SIZE_P(vecP)) { \
            __typeof__(*(vecP)) newVec = \
                (__typeof(*(vecP))) \
                    malloc(STRUCT_SIZE(**(vecP)) + SIZE_P(vecP) * sizeof(ID_P(vecP, 0))); \
            memcpy(newVec, *(vecP), STRUCT_SIZE(**(vecP))); \
            if (newVec == NULL) \
                do { fallbackAction; } while (0); \
            SIZE(newVec) += SIZE(newVec); \
            free(*(vecP)); \
            *(vecP) = newVec; \
        } \
        ID_P(vecP, CNT_P(vecP)) = (val); \
        ++CNT_P(vecP); \
    } while (0);

#define FREE_V(vec) do { free(*(vec)); *(vec) = NULL; } while (0)
