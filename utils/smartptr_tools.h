#include <assert.h>
#include "new_tools.h"

#define REFCNT_DEF size_t refCnt
#define REFCNT(t) ((t)->refCnt)

#define NEWSMRT(tar, T, onFail) do { \
    if (((tar) = NEW(T)) == NULL) do { onFail; } while (0); \
    else REFCNT(tar) = 1; \
} while (0)

#define CPYREF(src, dst) do { \
    (dst) = (src); \
    ++REFCNT(src); \
} while (0)

#define FREEREF_RET(t) if (t == NULL) return; assert(REFCNT(t)); if (--REFCNT(t) != 0) do { *t##P = NULL; return; } while (0)

#define IFFREE(t) assert(REFCNT(t)); if (--REFCNT(t) == 0)
