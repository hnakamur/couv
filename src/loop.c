#include "couv-private.h"

static int couv_default_loop(lua_State *L) {
  lua_pushlightuserdata(L, uv_default_loop());
  return 1;
}

static int couv_set_loop(lua_State *L) {
  lua_setfield(L, LUA_REGISTRYINDEX, COUV_LOOP_REGISTRY_KEY);
  return 0;
}

static int couv_get_loop(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, COUV_LOOP_REGISTRY_KEY);
  return 1;
}

uv_loop_t *couv_loop(lua_State *L) {
  uv_loop_t *loop;

  couv_get_loop(L);
  loop = lua_touserdata(L, -1);
  lua_pop(L, 1);
  return loop;
}

static int couv_run(lua_State *L) {
  uv_loop_t *loop;

  loop = couv_loop(L);
  uv_run(loop);
  return 0;
}

static int couv_run_once(lua_State *L) {
  uv_loop_t *loop;
  int r;

  loop = couv_loop(L);
  r = uv_run_once(loop);
  lua_pushnumber(L, r);
  return 1;
}

static const struct luaL_Reg loop_functions[] = {
  { "default_loop", couv_default_loop },
  { "get_loop", couv_get_loop },
  { "run", couv_run },
  { "run_once", couv_run_once },
  { "set_loop", couv_set_loop },
  { NULL, NULL }
};

int luaopen_couv_loop(lua_State *L) {
  couv_default_loop(L);
  couv_set_loop(L);

  luaL_register(L, NULL, loop_functions);
  return 1;
}
