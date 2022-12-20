#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pk_lexer.h"

enum ExprKind {
  EK_INTEGER, // Integer constant expression stuffedd in `ival`
  EK_DOUBLE, // Double constant expression stuffed in `dval`
  EK_STRING, // String constant expression stuffed in `str`

  EK_CONST, //Constant expression in constants array.
  EK_NON_RELOC, // Non relocatable expression stuffed in `info` register.
  EK_RELOC, // Relocatable expression the PC of the instruction is stored in `info` for modification later.
};

/* 
    this MUST match the ordering in OPCODES.
*/
enum Opr {
  OPR_ADD,
  OPR_MINUS,
  OPR_MULT,
  OPR_DIV
};

typedef struct pk_string pk_string;
typedef struct pk_value pk_value;
typedef struct pk_parser pk_parser;
typedef struct pk_funcstate pk_funcstate;

#define NO_JUMP -1

typedef struct expdesc {
  enum ExprKind kind;
  union {
    uint64_t ival;
    double dval;
    int info;
    pk_string* str;
  };
  int t;
  int f;
} expdesc;

int pk_code_exp2any(pk_funcstate* fs, expdesc* e);
void pk_code_infix(pk_parser* p, int type, expdesc* l);
void pk_code_postfix(pk_parser* p, int op, expdesc* e1, expdesc* e2);
bool pk_tonumeral(expdesc* e, pk_value* v);

int pk_code(pk_funcstate* fs, uint32_t op);
int pk_code_AsBx(pk_funcstate* fs, int o, int a, int bc);
int pk_code_ABx(pk_funcstate* fs, int o, int a, unsigned int bc);
int pk_code_ABCk(pk_funcstate* fs, int o, int a, int b, int c, int k);
int pk_code_k(pk_funcstate* fs, int reg, int k);

void pk_code_double(pk_funcstate* fs, int reg, double val);
void pk_code_integer(pk_funcstate* fs, int reg, long val);
int pk_code_checkstack(pk_funcstate* fs, int amt);
int pk_code_reserveregs(pk_funcstate* fs, int amt);
int pk_code_exp2next(pk_funcstate* fs, expdesc* e);
int pk_code_exp2any(pk_funcstate* fs, expdesc* e);
int pk_isKint(expdesc* e);


