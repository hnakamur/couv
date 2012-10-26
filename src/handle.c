#include "couv-private.h"


static void close_cb(uv_handle_t *handle) {
  lua_State *L;

  L = handle->data;

  switch (handle->type) {
  case UV_PROCESS:
    couv_clean_process_handle(L, (uv_process_t *)handle);
    break;
  case UV_TCP:
    couv_clean_tcp_handle(L, (uv_tcp_t *)handle);
    break;
  case UV_TIMER:
    couv_clean_timer_handle(L, (uv_timer_t *)handle);
    break;
  case UV_UDP:
    couv_clean_udp_handle(L, (uv_udp_t *)handle);
    break;
  case UV_TTY:
    couv_clean_tty_handle(L, (uv_tty_t *)handle);
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

  handle = couvL_checkudataclass(L, 1, COUV_HANDLE_MTBL_NAME);
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

  handle = couvL_checkudataclass(L, 1, COUV_HANDLE_MTBL_NAME);
  lua_pushboolean(L, uv_is_active(handle));
  return 1;
}

static int couv_is_closing(lua_State *L) {
  uv_handle_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_HANDLE_MTBL_NAME);
  lua_pushboolean(L, uv_is_closing(handle));
  return 1;
}

static int couv_ref(lua_State *L) {
  uv_handle_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_HANDLE_MTBL_NAME);
  uv_ref(handle);
  return 0;
}

static int couv_unref(lua_State *L) {
  uv_handle_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_HANDLE_MTBL_NAME);
  uv_unref(handle);
  return 0;
}

static int couv_handle_guess(lua_State *L) {
  uv_file file;
  uv_handle_type type;

  file = luaL_checkint(L, 1);
  type = uv_guess_handle(file);
  lua_pushnumber(L, type);
  return 1;
}

static const struct luaL_Reg handle_methods[] = {
  { "_close", couv_close },
  { "isActive", couv_is_active },
  { "isClosing", couv_is_closing },
  { "ref", couv_ref },
  { "unref", couv_unref },
  { NULL, NULL }
};

static const struct luaL_Reg handle_functions[] = {
  { "guess", couv_handle_guess },
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
  lua_newtable(L);
  set_handle_type_constants(L);
  couvL_setfuncs(L, handle_functions, 0);
  lua_setfield(L, -2, "Handle");

  couv_newmetatable(L, COUV_HANDLE_MTBL_NAME, NULL);
  couvL_setfuncs(L, handle_methods, 0);
  lua_setfield(L, -2, "_Handle");

  return 0;
}
