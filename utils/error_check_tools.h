#include <stdio.h>
#include <string.h>
#include <errno.h>


#define VERIFY_F(condition, operation, filename, rv, value, exit) \
do { \
    if (!(condition)) { \
        fprintf(stderr, "failed to %s %s: %s\n", \
            operation, filename, strerror(errno)); \
        rv = value; \
        exit; \
    } \
} while (0)


#define CHECK_F(condition, operation, filename, rv, value, label) \
VERIFY_F(condition, operation, filename, rv, value, goto label)


#define ROLLBACK_F(condition, operation, filename, rv, value) \
VERIFY_F(condition, operation, filename, rv, value, (void)0)

#define VERIFY(condition, message, setCode, exit) \
do { \
    if (!(condition)) { \
        fprintf(stderr, message "\n"); \
        setCode; \
        exit; \
    } \
} while (0)

#define CHECK(condition, message, setCode, label) \
VERIFY(condition, message, setCode, goto label)


#define ROLLBACK(condition, message, rv, value) \
VERIFY(condition, message, rv, value, (void)0)

#define CHK(condition, setCode, onFail) if (!(condition)) { setCode; onFail;}
