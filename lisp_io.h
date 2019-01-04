#pragma once

#include <stdio.h>

#include "models.h"

enum OpRetCode ReadExpression(FILE *file, struct Expression **out);

enum OpRetCode WriteObject(FILE *file, struct State *state, const struct Object *object);
