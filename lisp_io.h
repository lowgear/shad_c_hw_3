#pragma once

#include <stdio.h>

#include "models.h"

enum RetCode {
    IoOk,
    AllocFail,
    eOf,
    IoError,
};

enum RetCode ReadExpression(FILE *file, struct Expression **out);

enum RetCode WriteObject(FILE *file, const struct Object *object);