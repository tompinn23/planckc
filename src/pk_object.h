#pragma once

typedef enum {
  OBJECT_FUNC,
  OBJECT_CLOSURE,
  OBJECT_STRING,
  OBJECT_ARRAY,
  OBJECT_MAP,
} pk_object_type;

typedef struct pk_object {
  pk_object_type type;
  void (*destroy)(struct pk_object* object);
  struct pk_object* next;
} pk_object;

