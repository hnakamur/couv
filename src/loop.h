#ifndef _LOOP_H
#define _LOOP_H

#include <lauxlib.h>

int luaopen_yaluv_loop(lua_State *L);

#define LOOP_MTBL_NAME "luv.loop.Loop"
#define luv_checkloop(L, index) \
    (*(uv_loop_t **)luaL_checkudata(L, index, LOOP_MTBL_NAME))

#endif /* _LOOP_H */
