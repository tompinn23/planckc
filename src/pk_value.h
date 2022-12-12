#pragma once

#include <stdbool.h>

typedef enum {
  VALUE_NIL,
  VALUE_INT,
  VALUE_DBL,
  VALUE_BOOL,
  VALUE_OBJ,
} pk_val_type;

struct pk_object;

typedef struct pk_value {
  pk_val_type type;
  union {
    long l;
    double d;
    bool b;
    struct pk_object* o;
  };
} pk_value;
