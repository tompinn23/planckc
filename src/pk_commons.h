#pragma once

#if defined(PK_ASSERT)
#undef NDEBUG
#include <assert.h>
#define lua_assert(c) assert(c)
#endif

#if defined(lua_assert)
#define check_exp(c, e) (lua_assert(c), (e))
#else
#define lua_assert(c) ((void)0)
#define check_exp(c, e) (e)
#endif

#define cast(t, exp)	((t)(exp))

#define cast_void(i)	cast(void, (i))
#define cast_voidp(i)	cast(void *, (i))
#define cast_int(i)	cast(int, (i))
#define cast_uint(i)	cast(unsigned int, (i))
#define cast_uchar(i)	cast(unsigned char, (i))
#define cast_char(i)	cast(char, (i))
#define cast_charp(i)	cast(char *, (i))
#define cast_sizet(i)	cast(size_t, (i))
