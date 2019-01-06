#include "gtest/gtest.h"

extern "C" {
#include "models.h"
}

#define TEST_FREES \
TEST_FREE(FreeObj, Object) \
TEST_FREE(FreeExpr, Expression) \
TEST_FREE(FreeLazyExpr, LazyExpr)

#define TEST_FREE(FreeFunc, T) \
TEST(FreeFunc, CASE) { \
    T object; \
    object.refCnt = 2; \
    T *ref = &object; \
\
    FreeFunc(&ref); \
\
    CUR_CHECK; \
}

#define CUR_CHECK EXPECT_EQ(object.refCnt, 1);
#define CASE DecreaseRefCnt

TEST_FREES

#undef CUR_CHECK
#undef CASE

#define CUR_CHECK EXPECT_EQ(ref, nullptr);
#define CASE MakeRefNull

TEST_FREES

#undef CUR_CHECK
#undef CASE

