#pragma once

#define NEW(T) malloc(sizeof(T))

#define TRY_NEW(tar, T, onFail) do { \
    (tar) = NEW(T); \
    if ((tar) == NULL) onFail; \
} while (0)
