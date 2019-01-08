#include "iotools.h"

size_t NextTokenLen(FILE *file) {
    const long pos = ftell(file);
    int begin, end;
    fscanf(file, " %n%*[^\t\n\v\f\r ()]%n", &begin, &end);
    if (ferror(file))
        return 0;
    fseek(file, pos, SEEK_SET);
    if (ferror(file))
        return 0;
    return (size_t) (end - begin);
}
