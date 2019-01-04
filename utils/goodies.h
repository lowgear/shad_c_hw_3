#pragma once

#define NEW(T) malloc(sizeof(T))
#define NEWARR(T, S) malloc(sizeof(T) * (S))

#define TRY_NEW(tar, T, onFail) if (((tar) = NEW(T)) == NULL) onFail;

