#include "pk_lexer.h"
#include "memory.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
int pk_lexer_init(pk_lexer* l) {
  memset(l->source, 0, sizeof(l->source));
  l->size         = 0;
  l->line         = 0;
  l->current      = l->source;
  l->eof          = false;
  l->user         = NULL;
  l->user_destroy = NULL;
  return 0;
}

struct lexer_string_data {
  const char* source;
  size_t len;
  size_t offset;
};

int s_getc(pk_lexer* l) {
  struct lexer_string_data* data = l->user;
  if ((*(data->source + data->offset) == '\0')) {
    l->eof = true;
    return (*data->source + data->offset);
  }
  data->offset++;
  return *(data->source + data->offset - 1);
}
int s_peekc(pk_lexer* l, int far) {
  struct lexer_string_data* data = l->user;
  if (data->offset + far >= data->len) {
    return '\0';
  }
  return *(data->source + data->offset);
}
int s_reset(pk_lexer* l) {
  struct lexer_string_data* data = l->user;
  data->offset                   = 0;
  return 0;
}
void s_destroy(pk_lexer* l) { mem_free(l->user); }

int pk_lexer_init_buffer(pk_lexer* l, const char* buf, size_t len) {
  if (pk_lexer_init(l) < 0) {
    return -1;
  }
  struct lexer_string_data* data = mem_alloc(sizeof(*data));
  data->source                   = buf;
  data->len                      = len;
  data->offset                   = 0;
  l->user                        = data;
  l->getc                        = s_getc;
  l->peekc                       = s_peekc;
  l->reset                       = s_reset;
  l->user_destroy                = s_destroy;
  return 0;
}

int f_getc(pk_lexer* l) {
  FILE* fp = (FILE*)l->user;
  char ch;
  if (fread(&ch, sizeof(char), 1, fp) != 1) {
    l->eof = true;
    return '\0';
  }
  return ch;
}

int f_peekc(pk_lexer* l, int far) {
  FILE* fp   = l->user;
  size_t off = ftell(fp);
  fseek(fp, far, SEEK_CUR);
  char ch;
  if (fread(&ch, sizeof(char), 1, fp) != 1) {
    fseek(fp, off, SEEK_SET);
    return '\0';
  }
  fseek(fp, off, SEEK_SET);
  return ch;
}

int f_reset(pk_lexer* l) {
  FILE* fp = l->user;
  rewind(fp);
  return 0;
}

void f_destroy(pk_lexer* l) { fclose(l->user); }

int pk_lexer_init_file(pk_lexer* l, const char* file) {
  if (pk_lexer_init(l) < 0)
    return -1;
  FILE* fp = fopen(file, "r");
  if (!fp) {
    fprintf(stderr, "Failed to open script: %s - %s", file, strerror(errno));
    return -1;
  }
  l->user         = fp;
  l->getc         = f_getc;
  l->peekc        = f_peekc;
  l->reset        = f_reset;
  l->user_destroy = f_destroy;
  return 0;
}

static pk_token make_token(pk_lexer* l, pk_token_type type) {
  pk_token tok;
  tok.type           = type;
  tok.length         = (int)(l->current - l->source);
  tok.start          = mem_strndup(l->source, tok.length);
  tok.line           = l->line;
  l->source[l->size] = '\0';
  l->size            = 0;
  l->current         = l->source;
  return tok;
}

static pk_token error_token(pk_lexer* l, char* message) {
  pk_token tok;
  tok.type   = TOKEN_ERR;
  tok.start  = message;
  tok.length = strlen(message);
  tok.line   = l->line;
  return tok;
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }
static bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int advance(pk_lexer* l) {
  l->source[l->size++] = l->getc(l);
  l->current++;
  return *(l->current - 1);
}

static int peek(pk_lexer* l) { return l->peekc(l, 0); }
static int peek2(pk_lexer* l) { return l->peekc(l, 1); }

static bool match(pk_lexer* l, int ch) {
  if (l->eof)
    return false;
  if (*l->current != ch)
    return false;
  l->getc(l);
  return true;
}

static void skip_ws(pk_lexer* l) {
  for (;;) {
    int ch = peek(l);
    switch (ch) {
    case ' ':
    case '\r':
    case '\t':
      l->getc(l);
      break;
    case '\n':
      l->line++;
      l->getc(l);
      break;
    case '/':
      if (peek2(l) == '/') {
        while (peek(l) != '\n' && !l->eof)
          l->getc(l);
      } else {
        return;
      }
    default:
      return;
    }
  }
}

static pk_token make_string(pk_lexer* l) {
  while (peek(l) != '"' && !l->eof) {
    if (peek(l) == '\n')
      l->line++;
    advance(l);
  }
  if (l->eof)
    return error_token(l, "Unterminated String!");
  advance(l);
  return make_token(l, TOKEN_STRING);
}

static pk_token make_number(pk_lexer* l) {
  pk_token_type type = TOKEN_INT;
  while (is_digit(peek(l)))
    advance(l);
  if (peek(l) == '.' && is_digit(peek2(l))) {
    type = TOKEN_NUMBER;
    advance(l);
  }
  while (is_digit(peek(l)))
    advance(l);
  return make_token(l, TOKEN_INT);
}

static pk_token_type check_keyword(pk_lexer* l, int start, int len, const char* rest,
                                   pk_token_type type) {
  if (l->current - l->source == start + len && memcmp(l->source + start, rest, len) == 0) {
    return type;
  }
  return TOKEN_IDENT;
}
static pk_token_type ident_type(pk_lexer* l) {
  // clang-format off
  switch (l->source[0]) {
  case 'a': return check_keyword(l, 1, 4, "lass", TOKEN_CLASS);
  case 's': return check_keyword(l, 1, 4, "uper", TOKEN_SUPER);
  case 't':
    if(l->current - l->source > 1) {
      switch(l->source[1]) {
      case 'h': return check_keyword(l, 2, 2, "is", TOKEN_THIS);
      case 'r': return check_keyword(l, 2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;

  case 'f':
    if(l->current - l->source > 1) {
      switch(l->source[1]) {
      case 'a': return check_keyword(l, 2, 3, "lse", TOKEN_FALSE);
      case 'o': return check_keyword(l, 2, 1, "r", TOKEN_FOR);
      case 'n': return TOKEN_FUNC;
      }
    }
    break;
  case 'r': return check_keyword(l, 1, 5, "eturn", TOKEN_RETURN);

  case 'b': return check_keyword(l, 1, 4, "reak", TOKEN_BREAK);
  case 'i': return check_keyword(l, 1, 1, "f", TOKEN_IF);
  case 'e': return check_keyword(l, 1, 3, "lse", TOKEN_ELSE);
  case 'w': return check_keyword(l, 1, 4, "hile", TOKEN_WHILE);

  case 'n': return check_keyword(l, 1, 2, "il", TOKEN_NIL);
  case 'v': return check_keyword(l, 1, 2, "ar", TOKEN_VAR);
  }
  // clang-format on
  return TOKEN_IDENT;
}

static pk_token make_identifier(pk_lexer* l) {
  while (is_alpha(peek(l)) || is_digit(peek(l)))
    advance(l);
  return make_token(l, ident_type(l));
}

pk_token pk_lexer_next(pk_lexer* l) {
  skip_ws(l);
  if (l->eof)
    return make_token(l, TOKEN_EOF);

  int c = advance(l);

  if (is_alpha(c))
    return make_identifier(l);

  if (is_digit(c))
    return make_number(l);

  // clang-format off
  switch (c) {
    case '(': return make_token(l, TOKEN_L_PAREN);
    case ')': return make_token(l, TOKEN_R_PAREN);
    case '{': return make_token(l, TOKEN_L_BRACE);
    case '}': return make_token(l, TOKEN_R_BRACE);
    case ',': return make_token(l, TOKEN_COMMA);
    case '.': return make_token(l, TOKEN_DOT);
    case '-': return make_token(l, TOKEN_MINUS);
    case ';': return make_token(l, TOKEN_SEMICOLON);
    case '+': return make_token(l, TOKEN_PLUS);
    case '/': return make_token(l, TOKEN_SLASH);
    case '*': return make_token(l, TOKEN_STAR);
    case '&': return make_token(l, match(l, '&') ? TOKEN_AND : TOKEN_AMPERSAND);
    case '|': return make_token(l, match(l, '|') ? TOKEN_OR : TOKEN_PIPE);
    case '=': return make_token(l, match(l, '=') ? TOKEN_EQ : TOKEN_ASSIGN);
    case '!': return make_token(l, match(l, '=') ? TOKEN_NE : TOKEN_EXCL);
    case '<': return make_token(l, match(l, '=') ? TOKEN_LTEQ : TOKEN_LT);
    case '>': return make_token(l, match(l, '=') ? TOKEN_GTEQ : TOKEN_GT);
    case '"': return make_string(l);
  }
  // clang-format on
  return error_token(l, "Unknown Input");
}
