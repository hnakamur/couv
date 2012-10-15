#include "yaluv-private.h"

static int luv_default_loop(lua_State *L) {
  lua_pushlightuserdata(L, uv_default_loop());
  return 1;
}

static int luv_set_loop(lua_State *L) {
  lua_setfield(L, LUA_REGISTRYINDEX, LUV_LOOP_REGISTRY_KEY);
  return 0;
}

static int luv_get_loop(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, LUV_LOOP_REGISTRY_KEY);
  return 1;
}

uv_loop_t *luv_loop(lua_State *L) {
  uv_loop_t *loop;

  luv_get_loop(L);
  loop = lua_touserdata(L, -1);
  lua_pop(L, 1);
  return loop;
}

static int luv_run(lua_State *L) {
  uv_loop_t *loop;

  loop = luv_loop(L);
  uv_run(loop);
  return 0;
}

static int luv_run_once(lua_State *L) {
  uv_loop_t *loop;
  int r;

  loop = luv_loop(L);
  r = uv_run_once(loop);
  lua_pushnumber(L, r);
  return 1;
}

static const struct luaL_Reg loop_functions[] = {
  { "default_loop", luv_default_loop },
  { "get_loop", luv_get_loop },
  { "run", luv_run },
  { "run_once", luv_run_once },
  { "set_loop", luv_set_loop },
  { NULL, NULL }
};

int luaopen_yaluv_loop(lua_State *L) {
  luv_default_loop(L);
  luv_set_loop(L);

  luaL_register(L, NULL, loop_functions);
  return 1;
}
