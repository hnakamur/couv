#include "yaluv_priv.h"

#define LUV_UV_ERRNAME_GEN(val, name, s) case val: return #name;

const char *luv_uv_errname(int uv_errcode) {
  switch (uv_errcode) {
  UV_ERRNO_MAP(LUV_UV_ERRNAME_GEN)
  default: return "UNKNOWN";
  }
}

static const struct luaL_Reg functions[] = {
  { NULL, NULL }
};

int luaopen_yaluv(lua_State *L) {
  luaL_register(L, "yaluv", functions);

  luaopen_yaluv_loop(L);
  luaopen_yaluv_fs(L);

  return 1;
}
