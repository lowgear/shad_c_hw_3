#pragma once

#include <stdlib.h>

#define DEF_VECTOR(name, T) \
struct name { \
    size_t size; \
    T array[1]; \
}; \
\
struct name* New##name(size_t size) { \
    struct name* v = malloc(sizeof(struct name) + sizeof(T) * size - sizeof(T)); \
    if (v == NULL) \
        return NULL; \
    v->size = size; \
    return v; \
}
