#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pk_lexer.h"

enum ExprKind {
  EK_INTEGER,
  EK_DOUBLE,
  EK_STRING,
};

enum Opr {
  OPR_ADD   = TOKEN_PLUS,
  OPR_MINUS = TOKEN_MINUS,
  OPR_MULT  = TOKEN_STAR,
  OPR_DIV   = TOKEN_SLASH,
};

typedef struct pk_string pk_string;
typedef struct pk_value pk_value;
typedef struct pk_parser pk_parser;

typedef struct expdesc {
  enum ExprKind kind;
  union {
    uint64_t ival;
    double dval;
    pk_string* str;
  };
} expdesc;

void pk_emit_binary(pk_parser* p, int type, expdesc* l, expdesc* r);
bool pk_tonumeral(expdesc* e, pk_value* v);
