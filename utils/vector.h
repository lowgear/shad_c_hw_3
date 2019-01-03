#pragma once

#include <stdlib.h>
#include <string.h>

#define DEF_ARRAY(name, T) \
struct name { \
    size_t size; \
    T array[1]; \
}; \

#define INIT_ARR(v, sz, fallbackAction) do { \
    (v) = malloc(sizeof(*v) + sizeof((v)->array[0]) * (sz) - sizeof((v)->array[0])); \
    if ((v) == NULL) \
        fallbackAction; \
    (v)->size = (sz); \
} while(0)

#define DEF_VECTOR(name, T) \
    struct name { \
        size_t size; \
        size_t cnt; \
        T array[1]; \
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
        (vec) = (__typeof(vec))malloc(sizeof((*(vec))) + ((s) - 1) * sizeof(ID_P(&vec, 0))); \
        if ((vec) == NULL) \
            fallbackAction; \
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
                fallbackAction; \
            SIZE(newVec) += SIZE(newVec); \
            free(*(vecP)); \
            *(vecP) = newVec; \
        } \
        ID_P(vecP, CNT_P(vecP)) = (val); \
        ++CNT_P(vecP); \
    } while (0);

#define FREE_V(vec) free((vec));
