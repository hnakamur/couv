#include <uv.h>
#include <lauxlib.h>
#include "auxlib.h"

int luvL_is_in_mainthread(lua_State *L) {
  int is_mainthread = lua_pushthread(L);
  lua_pop(L, 1);
  return is_mainthread;
}

int luvL_hasmetatablename(lua_State *L, int index, const char *tname) {
  if (lua_getmetatable(L, index)) {
    luaL_getmetatable(L, tname);
    if (lua_rawequal(L, -1, -2)) {
      lua_pop(L, 2);
      return 1;
    }
    lua_pop(L, 2);
  }
  return 0;
}

#define LUV_UV_ERRNAME_GEN(val, name, s) case val: return #name;

const char *luvL_uv_errname(int uv_errcode) {
  switch (uv_errcode) {
  UV_ERRNO_MAP(LUV_UV_ERRNAME_GEN)
  default: return "UNKNOWN";
  }
}
