#include "goodies.h"

#define REFCNT_DEF size_t refCnt
#define REFCNT(t) ((t)->refCnt)

#define NEWSMRT(tar, T, onFail) do { \
    if (((tar) = NEW(T)) == NULL) {onFail;} \
    else REFCNT(tar) = 1; \
} while (0)

#define CPYREF(src, dst) do { \
    (dst) = (src); \
    ++REFCNT(src); \
} while (0)

#define FREEREF_RET(t) if (REFCNT(t) != 0 && (--REFCNT(t) != 0)) { *t##P = NULL; return; }

#define IFFREE(t) if (REFCNT(t) == 0 || (--REFCNT(t) == 0))