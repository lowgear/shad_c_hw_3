#include <stdio.h>

#include "gtest/gtest.h"

extern "C" {
#include "utils/iotools.h"
}

#define TEST_ON(testCase, input, expected) \
TEST(NextTokenLen, testCase) { \
        FILE *f; \
        char in[] = input; \
        f = fmemopen(in, strlen(in), "r"); \
        EXPECT_EQ(NextTokenLen(f), expected##U); \
        fclose(f); \
}

TEST_ON(Sanity, "kek", 3)

TEST_ON(DontCountWSpace1, " kek", 3)

TEST_ON(DontCountWSpace2, "\t\tkek\t\t", 3)

TEST_ON(DontCountWSpace3, "\r\rkekek\r\r", 5)

TEST_ON(DontCountWSpace4, "\v\vkek\v\v", 3)

TEST_ON(DontCountWSpace5, "\n\nkek\n\n", 3)

TEST_ON(DontCountWSpace6, "\f\fkek\f\f", 3)

TEST_ON(StopOnOpenBracket, "kek(", 3)

TEST_ON(StopOnCloseBracket, "kek)", 3)

#undef TEST_ON
