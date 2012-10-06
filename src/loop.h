#ifndef _LOOP_H
#define _LOOP_H

#include <lauxlib.h>
#include <uv.h>

int luaopen_yaluv_loop(lua_State *L);
uv_loop_t *luv_loop(lua_State *L);

#define LUV_LOOP_MTBL_NAME "luv.loop.Loop"
#define luv_checkloop(L, index) \
    (*(uv_loop_t **)luaL_checkudata(L, index, LUV_LOOP_MTBL_NAME))

#endif /* _LOOP_H */
