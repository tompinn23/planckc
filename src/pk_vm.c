#include "pk_vm.h"

#include <string.h>
#include "pk_lexer.h"
#include "pk_compile.h"

#include "pk_value.h"

#include "memory.h"

pk_value pk_vm_pop(pk_vm* vm) {
  return (pk_value){.type = VALUE_NIL, .l = 0 };
}

int pk_vm_call(pk_vm* vm, pk_function* o) {
  return 0;
}

int pk_vm_do_string(pk_vm* vm, const char* source) {
  pk_vm_loadbuffer(vm, source, strlen(source));
  pk_value v = pk_vm_pop(vm);
  return pk_vm_call(vm, (pk_function*)v.o);
}

int pk_vm_loadbuffer(pk_vm* vm, const char* buf, size_t len) {
  pk_lexer l;
  if(pk_lexer_init_buffer(&l, buf, len) < 0) {
    return -1;
  }
#ifdef PK_LEX_DEBUG
  pk_lexer_print(&l);
  pk_lexer_reset(&l);
#endif
  //pk_compiler* c = pk_compiler_new();
  //if(pk_compile(c, &l) < 0)
  //  return -1;

}

pk_object* pk_vm_allocate_object(pk_vm *vm, size_t size) {
  pk_object* obj = mem_alloc(size);
  obj->next = vm->objects;
  vm->objects = obj;
  return obj;
}

