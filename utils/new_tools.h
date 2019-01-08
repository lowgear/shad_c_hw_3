#pragma once

#define NEW(T) malloc(sizeof(T))
#define NEWARR(T, S) malloc(sizeof(T) * (S))

#define TRY_NEW(tar, T, onFail) if (((tar) = NEW(T)) == NULL) do { onFail; } while (0)
#define TRY_NEWARR(tar, T, S, onFail) if (((tar) = NEWARR(T, S)) == NULL) do { onFail; } while (0)
