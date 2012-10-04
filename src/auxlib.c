#include <uv.h>
#include "auxlib.h"

int luvL_is_in_mainthread(lua_State *L) {
  int is_mainthread = lua_pushthread(L);
  lua_pop(L, 1);
  return is_mainthread;
}

#define LUV_UV_ERRNAME_GEN(val, name, s) case val: return #name;

const char *luvL_uv_errname(int uv_errcode) {
  switch (uv_errcode) {
  UV_ERRNO_MAP(LUV_UV_ERRNAME_GEN)
  default: return "UNKNOWN";
  }
}
