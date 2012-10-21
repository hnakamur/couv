#include "couv-private.h"


static void close_cb(uv_handle_t *handle) {
  lua_State *L;

  L = handle->data;

  switch (handle->type) {
  case UV_TCP:
    couv_free_tcp_handle(L, (uv_tcp_t *)handle);
    break;
  case UV_TIMER:
    couv_free_timer_handle(L, (uv_timer_t *)handle);
    break;
  case UV_UDP:
    couv_free_udp_handle(L, (uv_udp_t *)handle);
    break;
  default:
    /* do nothing */
    break;
  }

  /* If we close handle from another thread, the thread for handle is not
   * yielded, so no need to resume.
   */
  if (lua_status(L) == LUA_YIELD)
    couv_resume(L, L, 0);
}

static int couv_close(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  uv_close(handle, close_cb);
  if (couvL_is_mainthread(L))
    return 0;
  else {
    lua_pop(L, 1);
    return lua_yield(L, 0);
  }
}

static int couv_is_active(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  lua_pushboolean(L, uv_is_active(handle));
  return 1;
}

static int couv_is_closing(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  lua_pushboolean(L, uv_is_closing(handle));
  return 1;
}

static int couv_ref(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  uv_ref(handle);
  return 0;
}

static int couv_unref(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  uv_unref(handle);
  return 0;
}

static const struct luaL_Reg handle_functions[] = {
  { "close", couv_close },
  { "is_active", couv_is_active },
  { "is_closing", couv_is_closing },
  { "ref", couv_ref },
  { "unref", couv_unref },
  { NULL, NULL }
};

static int set_handle_type_constants(lua_State *L) {
  couvL_SET_FIELD(L, UNKNOWN_HANDLE, number, UV_UNKNOWN_HANDLE);
#define XX(uc, lc) couvL_SET_FIELD(L, uc, number, UV_##uc);
  UV_HANDLE_TYPE_MAP(XX)
#undef XX
  couvL_SET_FIELD(L, FILE, number, UV_FILE);
  couvL_SET_FIELD(L, HANDLE_TYPE_MAX, number, UV_HANDLE_TYPE_MAX);
  return 0;
}

int luaopen_couv_handle(lua_State *L) {
  set_handle_type_constants(L);

  couvL_setfuncs(L, handle_functions, 0);
  return 1;
}
