#include "couv-private.h"

static int couv_hrtime(lua_State *L) {
  lua_pushnumber(L, uv_hrtime() / 1e9);
  return 1;
}

static const struct luaL_Reg functions[] = {
  { "hrtime", couv_hrtime },
  { NULL, NULL }
};

int luaopen_couv_native(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(functions) - 1);
  couvL_setfuncs(L, functions, 0);

  luaopen_couv_loop(L);

  luaopen_couv_buffer(L);
  luaopen_couv_fs(L);
  luaopen_couv_ipaddr(L);

  luaopen_couv_handle(L);
  luaopen_couv_pipe(L);
  luaopen_couv_stream(L);
  luaopen_couv_tcp(L);
  luaopen_couv_timer(L);
  luaopen_couv_tty(L);
  luaopen_couv_udp(L);

  return 1;
}
