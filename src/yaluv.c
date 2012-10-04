#include <lauxlib.h>
#include "yaluv.h"
#include "loop.h"
#include "fs.h"

static const struct luaL_Reg functions[] = {
  { NULL, NULL }
};

int luaopen_yaluv(lua_State *L) {
  luaL_register(L, "yaluv", functions);

  luaopen_yaluv_loop(L);
  luaopen_yaluv_fs(L);

  return 1;
}
