#include <lauxlib.h>
#include "auxlib.h"
#include "buffer.h"
#include "fs.h"
#include "ipaddr.h"
#include "loop.h"
#include "udp.h"
#include "yaluv.h"

static void close_cb(uv_handle_t* handle) {
  lua_State *L = (lua_State *)handle->loop->data;
  lua_resume(L, 0);
}

static int luv_close(lua_State *L) {
  uv_handle_t *handle = lua_touserdata(L, 1);
  uv_close(handle, close_cb);
  return lua_yield(L, 0);
}

static const struct luaL_Reg functions[] = {
  { "close", luv_close },
  { NULL, NULL }
};

int luaopen_yaluv(lua_State *L) {
  luaL_register(L, "yaluv", functions);

  luaopen_yaluv_loop(L);

  luaopen_yaluv_buffer(L);
  luaopen_yaluv_fs(L);
  luaopen_yaluv_ipaddr(L);
  luaopen_yaluv_udp(L);

  return 1;
}
