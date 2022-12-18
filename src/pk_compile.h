#pragma once

#include "pk_code.h"
#include "pk_lexer.h"
#include <stdint.h>

typedef struct pk_vm pk_vm;

typedef struct pk_funcstate {

  struct pk_funcstate* enclosing;
} pk_funcstate;

typedef struct pk_parser {
  pk_vm* vm;
  pk_lexer* l;
  pk_token previous;
  pk_token current;

  pk_funcstate* fs;

  bool panicking;
  bool error;
} pk_parser;

int pk_compile(pk_vm* vm, pk_lexer* l);
