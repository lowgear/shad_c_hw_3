#pragma once

#define GETARG(name, id) \
struct Object *name; \
rc = GetLazyExprVal(argv->array[id], state, &name); \
if (rc != Ok) return rc;

#define GETARGT(name, id, objType) \
GETARG(name, id) \
if (name->type != objType) return ArgTypeMismatch;
