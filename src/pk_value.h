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
    long ival;
    double dval;
    bool bval;
    struct pk_object* object;
  };
} pk_value;

typedef enum { F2Ieq, F2Ifloor, F2Iceil } FloatEqMode;

#define pk_IValue(n)                                                                               \
  (pk_value) {.type = VALUE_INT, .ival = n}
#define pk_DValue(n)                                                                               \
  (pk_value) {.type = VALUE_DBL, .dval = n}

int pk_value_dbl_to_int(double d, long* i, FloatEqMode mode);
