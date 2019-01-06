#include "iotools.h"

size_t NextTokenLen(FILE *file) {
    const long pos = ftell(file);
    int begin, end;
    fscanf(file, " %n%*[^\t\n\v\f\r ()]%n", &begin, &end);
    fseek(file, pos, SEEK_SET);
    return (size_t) (end - begin);
}