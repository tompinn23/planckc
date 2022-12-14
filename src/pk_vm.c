#include "pk_vm.h"

#include <stdio.h>
#include <string.h>

#include "pk_compile.h"
#include "pk_debug.h"
#include "pk_lexer.h"
#include "pk_opcode.h"

#include "memory.h"
#include "pk_value.h"

int pk_chunk_init(pk_chunk* chunk) {
  chunk->code     = NULL;
  chunk->size     = 0;
  chunk->capacity = 0;
  return 0;
}

void pk_chunk_free(pk_chunk* chunk) {
  mem_free(chunk->code);
  pk_chunk_init(chunk);
}

int pk_chunk_add(pk_chunk* chunk, Instruction ins) {
  if (chunk->capacity < chunk->size + 1) {
    int n = chunk->capacity * 2 + 8;
    chunk->code =
        mem_realloc(chunk->code, sizeof(Instruction) * chunk->capacity, sizeof(Instruction) * n);
    chunk->capacity = n;
  }
  chunk->code[chunk->size++] = ins;
  return chunk->size - 1;
}

int pk_constants_init(pk_constants* c) {
    c->capacity = c->count = 0;
  c->values               = NULL;
}
int pk_constants_add(pk_constants* c, pk_value v) {
  if (c->capacity < c->count + 1) {
    int new = c->capacity == 0 ? 8 : c->capacity * 2;
    c->values =
        mem_realloc(c->values, sizeof(pk_value) * c->capacity,
                    sizeof(pk_value) * new);
    c->capacity = new;
  }
  c->values[c->count++] = v;
  return c->count - 1;
}
void pk_constants_destroy(pk_constants* c) {
  c->capacity = 0;
  c->count     = 0;
  mem_free(c->values);
}

pk_function* pk_function_new(pk_vm* vm, pk_func_type type) {
  pk_function* f = (pk_function*)pk_vm_allocate_object(vm, sizeof(*f));
  pk_function_init(f, type);
  return f;
}

static void pk_func_obj_destroy(pk_object* o) { pk_function_destroy((pk_function*)o); }



int pk_function_init(pk_function* f, pk_func_type type) {
  f->obj.type     = OBJECT_FUNC;
  f->obj.destroy  = pk_func_obj_destroy;
  f->arity        = 0;
  f->locals       = 0;
  f->stacksize    = 0;
  f->maxstacksize = 0;
  f->upvaluecount = 0;
  f->enclosing    = NULL;
  pk_chunk_init(&f->chunk);
  return 0;
}

void pk_function_destroy(pk_function* f) { mem_free(f); }

pk_vm* pk_vm_new() {
  pk_vm* vm     = mem_alloc(sizeof(*vm));
  vm->objects   = NULL;
  vm->stack_top = vm->stack;
  return vm;
}

pk_value pk_vm_pop(pk_vm* vm) { return (pk_value){.type = VALUE_NIL, .ival = 0}; }

void pk_vm_push(pk_vm* vm, pk_value v) {
  *vm->stack_top = v;
  vm->stack_top++;
}

int pk_vm_call(pk_vm* vm, pk_function* o) { return 0; }

int pk_vm_do_string(pk_vm* vm, const char* source) {
  pk_vm_loadbuffer(vm, source, strlen(source));
  pk_value v = pk_vm_pop(vm);
  return pk_vm_call(vm, (pk_function*)v.object);
}

int pk_vm_loadbuffer(pk_vm* vm, const char* buf, size_t len) {
  pk_lexer l;
  if (pk_lexer_init_buffer(&l, buf, len) < 0) {
    return -1;
  }
#ifdef PK_LEX_DEBUG
  pk_lexer_print(&l);
  pk_lexer_reset(&l);
#endif
  if (pk_compile(vm, &l) < 0)
    return -1;
  return 0;
}

pk_object* pk_vm_allocate_object(pk_vm* vm, size_t size) {
  pk_object* obj = mem_alloc(size);
  obj->next      = vm->objects;
  vm->objects    = obj;
  return obj;
}

int pk_vm_do_chunk(pk_vm* vm, pk_chunk* chunk) {
  pk_cf callframe = (pk_cf){.base = vm->stack, .top = vm->stack};
  Instruction* ip = chunk->code;
  while (1) {
    printf("stack\n");
    int i = 0;
    char buf[256];
    for (pk_value* v = vm->stack; v < vm->stack_top; v++) {
      pk_debug_print_val(v, buf, sizeof(buf));
      printf("[%d] [%s] %s\n", i++, pk_debug_value_type(v), buf);
    }
    switch (GET_OPCODE(*ip)) {
    case OP_MOVE:
      break;
    case OP_LOADI: {
      int k = GETARG_sBx(*ip);
      pk_vm_push(vm, (struct pk_value){.type = VALUE_INT, .ival = k});
      break;
    }
    case OP_RETURN: {
      uint8_t idx = GETARG_A(*ip);

      return 1;
      break;
    }
    default:
      printf("unhandled instruction: %s\n", pk_opnames[GET_OPCODE(*ip)]);
    }
    ip++;
  }
  return 0;
}
