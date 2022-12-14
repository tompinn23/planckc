#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef enum {
  TOKEN_L_PAREN,
  TOKEN_R_PAREN,
  TOKEN_L_BRACE,
  TOKEN_R_BRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,
  TOKEN_ASSIGN,
  TOKEN_EXCL,
  TOKEN_AMPERSAND,
  TOKEN_PIPE,

  TOKEN_NE,
  TOKEN_EQ,
  TOKEN_GT,
  TOKEN_LT,
  TOKEN_GTEQ,
  TOKEN_LTEQ,
  TOKEN_AND,
  TOKEN_OR,

  TOKEN_IDENT,
  TOKEN_STRING,
  TOKEN_INT,
  TOKEN_NUMBER,

  TOKEN_CLASS,
  TOKEN_SUPER,
  TOKEN_THIS,

  TOKEN_FUNC,
  TOKEN_RETURN,
  TOKEN_BREAK,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FOR,

  TOKEN_NIL,
  TOKEN_VAR,
  TOKEN_TRUE,
  TOKEN_FALSE,

  TOKEN_ERR,
  TOKEN_EOF
} pk_token_type;



typedef struct pk_token {
  pk_token_type type;
  char *start;
  int length;
  int line;
} pk_token;

typedef struct pk_lexer {
  char source[512];
  int size;
  const char* current;
  int line;
  bool eof;
  void* user;
  int (*getc)(struct pk_lexer* l);
  int (*peekc)(struct pk_lexer* l, int far);
  int (*reset)(struct pk_lexer* l);
  void (*user_destroy)(struct pk_lexer *l);
} pk_lexer;

int pk_lexer_init(pk_lexer* l);

int pk_lexer_init_buffer(pk_lexer* l, const char* buf, size_t len);
int pk_lexer_init_file(pk_lexer* l, const char* file);

pk_token pk_lexer_next(pk_lexer* l);
