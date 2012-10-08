#ifndef _BUFFER_H
#define _BUFFER_H

#include <lua.h>

int luaopen_yaluv_buffer(lua_State *L);

typedef struct luv_buffer_s {
  int length;
  int buf_ref;
  char *buf;
} luv_buffer_t;

#define LUV_BUFFER_MTBL_NAME "luv.Buffer"
#define luv_checkbuffer(L, index) \
    (luv_buffer_t *)luaL_checkudata(L, index, LUV_BUFFER_MTBL_NAME)
const char *luv_checkbuforstr(lua_State *L, int index, size_t *length);

#define luv_argcheckindex(L, arg_index, index, min, max) \
  luaL_argcheck(L, min <= index && index <= max, arg_index, \
      "index out of range");

#endif /* _BUFFER_H */
