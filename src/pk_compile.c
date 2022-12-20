#include "pk_compile.h"
#include "pk_vm.h"
#include "pk_debug.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  PREC_NONE,
  PREC_ASSIGN,
  PREC_OR,
  PREC_AND,
  PREC_EQ,
  PREC_COMP,
  PREC_TERM,
  PREC_FACTOR,
  PREC_UNARY,
  PREC_CALL,
  PREC_PRIMARY
};

typedef void (*parse_fn)(pk_parser* p, expdesc* v, bool ca);
struct pkP_rule {
  parse_fn prefix;
  parse_fn infix;
  int precedence;
};

static struct pkP_rule* pk_get_rule(pk_token_type type);

static void error_at(pk_parser* p, pk_token* t, const char* message) {
  if (p->panicking)
    return;
  p->panicking = true;
  fprintf(stderr, "[%d] err", t->line);
  if (t->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (t->type == TOKEN_ERR) {

  } else {
    fprintf(stderr, " at '%.*s'", t->length, t->start);
  }
  fprintf(stderr, ": %s\n", message);
  p->error = true;
}
static void error_current(pk_parser* p, const char* message) { error_at(p, &p->current, message); }
static void error(pk_parser* p, const char* message) { error_at(p, &p->previous, message); }

static void advance(pk_parser* p) {
  p->previous = p->current;
  for (;;) {
    p->current = pk_lexer_next(p->l);
    if (p->current.type != TOKEN_ERR) {
      break;
    }
  }
}

static void consume(pk_parser* p, pk_token_type type, const char* err) {
  if (p->current.type == type) {
    advance(p);
    return;
  }
  error_current(p, err);
}

static bool check(pk_parser* p, pk_token_type type) { return p->current.type == type; }

static bool match(pk_parser* p, pk_token_type type) {
  if (!check(p, type))
    return false;
  advance(p);
  return true;
}

static enum Opr opr_token[] = {[TOKEN_MINUS] = OPR_MINUS,
                               [TOKEN_PLUS]  = OPR_ADD,
                               [TOKEN_STAR]  = OPR_MULT,
                               [TOKEN_SLASH] = OPR_DIV};

static void precedence(pk_parser* p, expdesc* e, int prec) {
  advance(p);
  parse_fn prefixR = pk_get_rule(p->previous.type)->prefix;
  if (prefixR == NULL) {
    error(p, "Expected expression");
    return;
  }
  bool ca = prec <= PREC_ASSIGN;
  prefixR(p, e, ca);
  while (prec < pk_get_rule(p->current.type)->precedence) {
    expdesc v2;
    advance(p);
    parse_fn infix = pk_get_rule(p->previous.type)->infix;
    enum Opr op         = opr_token[p->previous.type];
    pk_code_infix(p, p->previous.type, e);
    infix(p, &v2, ca);
    pk_code_postfix(p, op, e, &v2);
  }
  if (ca && match(p, TOKEN_ASSIGN)) {
    error(p, "Invalid assignment target.");
  }
}

static void unary(pk_parser* p, expdesc* e, bool ca) {}

static void binary(pk_parser* p, expdesc* e, bool ca) {
  pk_token_type type    = p->previous.type;
  struct pkP_rule* rule = pk_get_rule(type);
  precedence(p, e, rule->precedence + 1);
  // TODO: constant folding
}
static void grouping(pk_parser* p, expdesc* e, bool ca) {}
static void and_(pk_parser* p, expdesc* e, bool ca) {}
static void or_(pk_parser* p, expdesc* e, bool ca) {}

static void string(pk_parser* p, expdesc* e, bool ca) {}
static void literal(pk_parser* p, expdesc* e, bool ca) {}
static void number(pk_parser* p, expdesc* e, bool ca) {
  if (p->previous.type == TOKEN_NUMBER) {
    double val = strtod(p->previous.start, NULL);
    e->kind    = EK_DOUBLE;
    e->dval    = val;
  } else if (p->previous.type == TOKEN_INT) {
    long val = strtol(p->previous.start, NULL, 10);
    e->kind  = EK_INTEGER;
    e->ival  = val;
  } else {
    error(p, "Expected number type");
  }
}

static void variable(pk_parser* p, expdesc* e, bool ca) {}
static void call(pk_parser* p, expdesc* e, bool ca) {}

static void expression(pk_parser* p, expdesc* e) { precedence(p, e, PREC_ASSIGN); }

static void expression_statement(pk_parser* p) {
  expdesc e;

  expression(p, &e);
  consume(p, TOKEN_SEMICOLON, "Expecting ';' after expression.");
}

static void statement(pk_parser* p) {
  if (match(p, TOKEN_L_BRACE)) {
  } else {
    expression_statement(p);
  }
}

static int parse_variable(pk_parser* p, const char* err) { consume(p, TOKEN_IDENT, err);
  //declare_variable(p);
  if (p->fs->scope > 0) {
      return 0;
  }
  
}

static void var_declaration(pk_parser* p) { int global = parse_variable(p, "Expected a variable name"); }

static void synchronize(pk_parser* p) {
  p->panicking = false;
  while (p->current.type != TOKEN_EOF) {
    if (p->previous.type == TOKEN_SEMICOLON) {
      return;
    }
    switch (p->current.type) {
    case TOKEN_CLASS:
    case TOKEN_FUNC:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_RETURN:
      return;
    default:;
    }
    advance(p);
  }
}

static void declaration(pk_parser* p) {
  if (match(p, TOKEN_FUNC)) {
  } else if (match(p, TOKEN_VAR)) {
    var_declaration(p);
  } else {
    statement(p);
  }

  if (p->panicking) {
    synchronize(p);
  }
}

static struct pkP_rule rules[] = {
    [TOKEN_L_PAREN]   = {grouping, call,   PREC_CALL  },
    [TOKEN_R_PAREN]   = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_L_BRACE]   = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_R_BRACE]   = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_COMMA]     = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_DOT]       = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_MINUS]     = {unary,    binary, PREC_TERM  },
    [TOKEN_PLUS]      = {NULL,     binary, PREC_TERM  },
    [TOKEN_SEMICOLON] = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_SLASH]     = {NULL,     binary, PREC_FACTOR},
    [TOKEN_STAR]      = {NULL,     binary, PREC_FACTOR},
    [TOKEN_EXCL]      = {unary,    NULL,   PREC_NONE  },
    [TOKEN_NE]        = {NULL,     binary, PREC_EQ    },
    [TOKEN_ASSIGN]    = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_EQ]        = {NULL,     binary, PREC_EQ    },
    [TOKEN_GT]        = {NULL,     binary, PREC_COMP  },
    [TOKEN_GTEQ]      = {NULL,     binary, PREC_COMP  },
    [TOKEN_LT]        = {NULL,     binary, PREC_COMP  },
    [TOKEN_LTEQ]      = {NULL,     binary, PREC_COMP  },
    [TOKEN_IDENT]     = {variable, NULL,   PREC_NONE  },
    [TOKEN_STRING]    = {string,   NULL,   PREC_NONE  },
    [TOKEN_NUMBER]    = {number,   NULL,   PREC_NONE  },
    [TOKEN_INT]       = {number,   NULL,   PREC_NONE  },
    [TOKEN_AND]       = {NULL,     and_,   PREC_NONE  },
    [TOKEN_CLASS]     = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_ELSE]      = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_FALSE]     = {literal,  NULL,   PREC_NONE  },
    [TOKEN_FOR]       = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_FUNC]      = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_IF]        = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_NIL]       = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_OR]        = {NULL,     or_,    PREC_NONE  },
 //[TOKEN_PRINT] = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_RETURN] = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_SUPER]  = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_THIS]   = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_TRUE]   = {literal,  NULL,   PREC_NONE  },
    [TOKEN_VAR]    = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_WHILE]  = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_ERR]    = {NULL,     NULL,   PREC_NONE  },
    [TOKEN_EOF]    = {NULL,     NULL,   PREC_NONE  },
};

int pk_compile_nvarstack(int reg) {
    return 0;
}

struct pkP_rule* pk_get_rule(pk_token_type type) { return &rules[type]; }

int pk_funcstate_init(pk_vm* vm, pk_funcstate* fs) { fs->enclosing = NULL;
  fs->freereg   = 0;
  fs->func = pk_function_new(vm, FUNC_SCRIPT);
  fs->scope = 0;
}

int pk_compile(pk_vm* vm, pk_lexer* l) {
  pk_parser p;
  p.vm = vm;
  p.l  = l;
  p.error = p.panicking = false;
  pk_funcstate fs;
  pk_funcstate_init(vm, &fs);
  p.fs                  = &fs;
  advance(&p);
  while (!match(&p, TOKEN_EOF)) {
    declaration(&p);
  }
  if (p.error) {
    printf("%s\n", p.previous.start);
  }
  else {
    pk_chunk_debug(&fs.func->chunk);
  }
  return 0;
}
