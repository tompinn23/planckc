#pragma once

#include <stdint.h>
#include <stddef.h>

#include "pk_object.h"
#include "pk_value.h"
#include "pk_opcode.h"
#define MAX_STACK 512

typedef struct pk_chunk {
  Instruction* code;
  size_t size, capacity;
} pk_chunk;

typedef enum { FUNC_FUNC, FUNC_SCRIPT } pk_func_type;

typedef struct pk_function {
  pk_object obj;
  pk_func_type type;
  int arity;
  int locals;
  int stacksize;
  int upvaluecount;
  pk_chunk chunk;
  struct pk_function* enclosing;
} pk_function;

typedef struct pk_cf {
  pk_value* base;
  pk_value* top;
} pk_cf;


typedef struct pk_vm {
  pk_object* objects;
  pk_value stack[MAX_STACK];
  pk_value* stack_top;
  pk_cf* callframe;
} pk_vm;

int pk_chunk_init(pk_chunk* chunk);
void pk_chunk_free(pk_chunk* chunk);
void pk_chunk_add(pk_chunk* chunk, Instruction ins);

pk_function* pk_function_new(pk_vm* vm, pk_func_type type);
int pk_function_init(pk_function* func, pk_func_type type);
void pk_function_destroy(pk_function* f);

pk_vm* pk_vm_new();
int pk_vm_do_string(pk_vm* vm, const char* source);
int pk_vm_do_file(pk_vm* vm, const char* file);
int pk_vm_loadbuffer(pk_vm* vm, const char* buf, size_t len);
int pk_vm_do_chunk(pk_vm* vm, pk_chunk* chunk);
pk_object* pk_vm_allocate_object(pk_vm* vm, size_t size);
