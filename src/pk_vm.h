#pragma once

#include <stdint.h>
#include <stddef.h>

#include "pk_object.h"

typedef struct pk_vm {
  pk_object* objects;
} pk_vm;

int pk_vm_do_string(pk_vm* vm, const char* source);
int pk_vm_do_file(pk_vm* vm, const char* file);
int pk_vm_loadbuffer(pk_vm* vm, const char* buf, size_t len);

pk_object* pk_vm_allocate_object(pk_vm* vm, size_t size);
