#pragma once

#include <stdint.h>
#include <stddef.h>

#define mem_alloc(size) mem_realloc(NULL, 0, size)
#define mem_free(ptr) mem_realloc(ptr, 0, 0)

void* mem_realloc(void* ptr, size_t old, size_t new);

char* mem_strdup(const char* str);
char* mem_strndup(const char* str, int len);

