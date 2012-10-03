#include "yaluv_priv.h"

static const struct luaL_Reg functions[] = {
  { NULL, NULL }
};

int luaopen_yaluv(lua_State *L) {
  luaL_register(L, "yaluv", functions);
  return 1;
}
