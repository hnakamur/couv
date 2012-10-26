#include "couv-private.h"

static uv_timer_t *couv_new_timer_handle(lua_State *L) {
  uv_timer_t *handle;

  handle = lua_newuserdata(L, sizeof(uv_timer_t));
  if (!handle)
    return NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, COUV_TIMER_MTBL_NAME);
  lua_setmetatable(L, -2);

  return handle;
}

void couv_clean_timer_handle(lua_State *L, uv_timer_t *handle) {
  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_TIMER_CB_REG_KEY(handle));
  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
}

static int couv_timer_create(lua_State *L) {
  uv_loop_t *loop;
  uv_timer_t *handle;
  int r;

  handle = couv_new_timer_handle(L);
  if (!handle)
    return 0;

  loop = couv_loop(L);
  r = uv_timer_init(loop, handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }
  handle->data = L;
  return 1;
}

static void timer_cb(uv_timer_t *handle, int status) {
  lua_State *L;

  L = handle->data;
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_TIMER_CB_REG_KEY(handle));
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
  if (status < 0) {
    lua_pushstring(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    lua_call(L, 2, 0);
  } else {
    lua_call(L, 1, 0);
  }
}

static int timer_start(lua_State *L) {
  uv_timer_t *handle;
  int64_t timeout;
  int64_t repeat;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TIMER_MTBL_NAME);
  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_pushvalue(L, 2);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_TIMER_CB_REG_KEY(handle));
  timeout = luaL_optinteger(L, 3, 0);
  repeat = luaL_optinteger(L, 4, 0);
  r = uv_timer_start(handle, timer_cb, timeout, repeat);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  lua_pushvalue(L, 1);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
  return 0;
}

static int timer_stop(lua_State *L) {
  uv_timer_t *handle;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TIMER_MTBL_NAME);
  r = uv_timer_stop(handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int timer_again(lua_State *L) {
  uv_timer_t *handle;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TIMER_MTBL_NAME);
  r = uv_timer_again(handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int timer_set_repeat(lua_State *L) {
  uv_timer_t *handle;
  int64_t repeat;

  handle = couvL_checkudataclass(L, 1, COUV_TIMER_MTBL_NAME);
  repeat = luaL_checkinteger(L, 2);
  uv_timer_set_repeat(handle, repeat);
  return 0;
}

static int timer_get_repeat(lua_State *L) {
  uv_timer_t *handle;
  int64_t repeat;

  handle = couvL_checkudataclass(L, 1, COUV_TIMER_MTBL_NAME);
  repeat = uv_timer_get_repeat(handle);
  lua_pushnumber(L, repeat);
  return 1;
}

static const struct luaL_Reg timer_functions[] = {
  { "timer_again", timer_again },
  { "timer_create", couv_timer_create },
  { "timer_get_repeat", timer_get_repeat },
  { "timer_set_repeat", timer_set_repeat },
  { "timer_stop", timer_stop },
  { "timer_start", timer_start },
  { NULL, NULL }
};

int luaopen_couv_timer(lua_State *L) {
  couv_newmetatable(L, COUV_TIMER_MTBL_NAME, COUV_HANDLE_MTBL_NAME);
  lua_pop(L, 1);

  couvL_setfuncs(L, timer_functions, 0);
  return 1;
}
