#include "iotools.h"

size_t NextStrLen(FILE *file) {
    const long pos = ftell(file);
    int begin, end;
    fscanf(file, " %n%*s%n", &begin, &end);
    fseek(file, pos, SEEK_SET);
    // todo errors
    return (size_t) (end - begin);
}