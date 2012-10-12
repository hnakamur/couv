#include <lauxlib.h>
#include <uv.h>
#include "auxlib.h"
#include "loop.h"

#define LOOP_REGISTRY_KEY "luv.loop"

static int loop_default(lua_State *L) {
  uv_loop_t **loop = (uv_loop_t **)lua_newuserdata(L, sizeof(uv_loop_t *));
  *loop = uv_default_loop();
  luaL_getmetatable(L, LUV_LOOP_MTBL_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int loop_set(lua_State *L) {
  uv_loop_t *loop;
  
  loop = *(uv_loop_t **)lua_touserdata(L, -1);
  loop->data = L;
  lua_setfield(L, LUA_REGISTRYINDEX, LOOP_REGISTRY_KEY);
  return 0;
}

static int loop_get(lua_State *L) {
  lua_getfield(L, LUA_REGISTRYINDEX, LOOP_REGISTRY_KEY);
  luaL_getmetatable(L, LUV_LOOP_MTBL_NAME);
  lua_setmetatable(L, -2);
  return 1;
}

uv_loop_t *luv_loop(lua_State *L) {
  uv_loop_t *loop;
  loop_get(L);
  loop = *(uv_loop_t **)lua_touserdata(L, -1);
  luaL_getmetatable(L, LUV_LOOP_MTBL_NAME);
  lua_setmetatable(L, -2);
  lua_pop(L, 1);
  return loop;
}

static int loop_run(lua_State *L) {
  uv_loop_t *loop = luv_checkloop(L, 1);
  uv_run(loop);
  return 0;
}

static int loop_run_once(lua_State *L) {
  uv_loop_t *loop = luv_checkloop(L, 1);
  int r = uv_run_once(loop);
  lua_pushnumber(L, r);
  return 1;
}

static const struct luaL_Reg loop_methods[] = {
  { "run", loop_run },
  { "run_once", loop_run_once },
  { NULL, NULL }
};

static const struct luaL_Reg loop_functions[] = {
  { "default", loop_default },
  { "get", loop_get },
  { "set", loop_set },
  { NULL, NULL }
};

int luaopen_yaluv_loop(lua_State *L) {
  loop_default(L);
  loop_set(L);

  lua_createtable(L, 0, ARRAY_SIZE(loop_functions) - 1);
  luaL_register(L, NULL, loop_functions);

  luaL_newmetatable(L, LUV_LOOP_MTBL_NAME);
  luaL_register(L, NULL, loop_methods);
  lua_setfield(L, -1, "__index");

  lua_setfield(L, -2, "loop");
  return 1;
}
