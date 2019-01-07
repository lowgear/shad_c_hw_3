#include <string.h>
#include <ctype.h>

#include "strtools.h"

bool ParseInt32(const char *str, int32_t *out) {
    const size_t len = strlen(str);
    if (len < 1)
        return false;
    int64_t res = 0;
    bool neg = (str[0] == '-');
    if (!(isdigit(str[0])
          || ((str[0] == '-' || str[0] == '+') && len > 1)))
        return false;
    for (size_t i = (isdigit(str[0]) ? 0 : 1); i < len; ++i) {
        if (!isdigit(str[i]))
            return false;
        res = res * 10 + str[i] - '0';

        if (res > INT32_MAX || (neg && -res < INT32_MIN))
            return false;
    }
    if (out == NULL)
        return true;
    if (neg)
        res = -res;
    *out = (int32_t) res;
    return true;
}
