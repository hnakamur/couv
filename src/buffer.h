#ifndef _BUFFER_H
#define _BUFFER_H

#include <lua.h>
#include <uv.h>

int luaopen_yaluv_buffer(lua_State *L);

#define LUV_BUFFER_MTBL_NAME "luv.Buffer"
#define luv_checkbuf(L, index) \
    (uv_buf_t *)luaL_checkudata(L, index, LUV_BUFFER_MTBL_NAME)
uv_buf_t luv_checkbuforstr(lua_State *L, int index);

/* NOTE: you must free the result buffers array. */
uv_buf_t *luv_checkbuforstrtable(lua_State *L, int index, size_t *buffers_cnt);

#define luv_argcheckindex(L, arg_index, index, min, max) \
  luaL_argcheck(L, (int)min <= index && index <= (int)max, arg_index, \
      "index out of range");

#endif /* _BUFFER_H */
