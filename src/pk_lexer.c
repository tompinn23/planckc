#include "pk_lexer.h"
#include "memory.h"

#include <string.h>
#include <stdio.h>

int pk_lexer_init(pk_lexer* l) {
  memset(l->source, 0, sizeof(l->source));
  l->line = 0;
  l->current = l->source;
  l->eof = false;
  l->user = NULL;
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
  if((*(data->source + data->offset) == '\0')) {
    l->eof = true;
    return (*data->source + data->offset);
  }
  data->offset++;
  return *(data->source + data->offset - 1);
}
int s_peekc(pk_lexer* l, int far) {
  struct lexer_string_data* data = l->user;
  if(data->offset + far >= data->len) {
    return '\0';
  }
  return *(data->source + data->offset);
}
int s_reset(pk_lexer* l) {
  struct lexer_string_data* data = l->user;
  data->offset = 0;
  return 0;
}
void s_destroy(pk_lexer* l) {
  mem_free(l->user);
}

int pk_lexer_init_buffer(pk_lexer *l, const char *buf, size_t len) {
  if(pk_lexer_init(l) < 0) {
    return -1;
  }
  struct lexer_string_data* data = mem_alloc(sizeof(*data));
  data->source = buf;
  data->len = len;
  data->offset = 0;
  l->user = data;
  l->getc = s_getc;
  l->peekc = s_peekc;
  l->reset = s_reset;
  l->user_destroy = s_destroy;
  return 0;
}

int f_getc(pk_lexer* l) {
  FILE* fp = (FILE*)l->user;
  char ch;
  if(fread(&ch, sizeof(char), 1, fp) != -1) {
    l->eof = true;
    return '\0';
  }
  return ch;
}

int f_peekc(pk_lexer* l, int far) {
  FILE* fp = l->user;
  size_t off = ftell(fp);
  fseek(fp, far, SEEK_CUR);
  char ch;
  if(fread(&ch, sizeof(char), 1, fp) != -1) {
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

void f_destroy(pk_lexer* l) {
  fclose(l->user);
}

int pk_lexer_init_file(pk_lexer* l, const char* file) {

}
