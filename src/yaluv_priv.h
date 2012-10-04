#ifndef _YALUV_PRIV_H
#define _YALUV_PRIV_H

#include <stdlib.h>
#include <string.h>

#include <lauxlib.h>
#include <uv.h>
#include "yaluv.h"

int luaopen_yaluv_loop(lua_State *L);
int luaopen_yaluv_fs(lua_State *L);

#define LOOP_MTBL_NAME "luv.loop.Loop"
#define luv_checkloop(L, index) \
    ((uv_loop_t **)luaL_checkudata(L, index, LOOP_MTBL_NAME))

const char *luv_uv_errname(int uv_errcode);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#endif /* _YALUV_PRIV_H */
