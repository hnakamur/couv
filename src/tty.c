#include "couv-private.h"

static uv_tty_t *couv_new_tty_handle(lua_State *L) {
  couv_tty_t *w_handle;
  uv_tty_t *handle;

  w_handle = lua_newuserdata(L, sizeof(couv_tty_t));
  if (!w_handle)
    return NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, COUV_TTY_MTBL_NAME);
  lua_setmetatable(L, -2);

  handle = &w_handle->handle;

  if (!couvL_is_mainthread(L))
    couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));
  return handle;
}

void couv_clean_tty_handle(lua_State *L, uv_tty_t *handle) {
  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_LISTEN_CB_REG_KEY(handle));
}

static int tty_new(lua_State *L) {
  uv_loop_t *loop;
  uv_tty_t *handle;
  uv_file fd;
  int readable;
  couv_stream_handle_data_t *hdata;
  int r;

  fd = luaL_checkint(L, 1);
  readable = luaL_checkint(L, 2);

  handle = couv_new_tty_handle(L);
  if (!handle)
    return 0;

  loop = couv_loop(L);
  r = uv_tty_init(loop, handle, fd, readable);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }

  handle->data = L;
  hdata = couv_get_stream_handle_data((uv_stream_t *)handle);
  hdata->is_yielded_for_input = 0;
  ngx_queue_init(&hdata->input_queue);

  lua_pushvalue(L, -1);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
  return 1;
}

static int tty_open_fd(lua_State *L) {
  int type;
  int fd;
#ifdef _WIN32
  HANDLE handle;
  const char *name;
#else
  int flags;
#endif

  type = luaL_checkint(L, 1);
#ifdef _WIN32
  if (type == 0)
    name = "conin$";
  else if (type == 1)
    name = "conout$";
  else
    return luaL_argerror(L, 1, "must be 0(ttyin) or 1(ttyout)");

  handle = CreateFileA(name,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
  fd = _open_osfhandle((intptr_t) handle, 0);
#else
  if (type == 0)
    flags = O_RDONLY;
  else if (type == 1)
    flags = O_WRONLY;
  else
    return luaL_argerror(L, 1, "must be 0(ttyin) or 1(ttyout)");
  fd = open("/dev/tty", flags, 0);
#endif
  lua_pushnumber(L, fd);
  return 1;
}

static int tty_set_mode(lua_State *L) {
  uv_tty_t *handle;
  int mode;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TTY_MTBL_NAME);
  mode = luaL_checkint(L, 2);
  r = uv_tty_set_mode(handle, mode);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int tty_reset_mode(lua_State *L) {
  uv_tty_reset_mode();
  return 0;
}

static int tty_get_winsize(lua_State *L) {
  uv_tty_t *handle;
  int width;
  int height;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TTY_MTBL_NAME);
  r = uv_tty_get_winsize(handle, &width, &height);
  if (r) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

static const struct luaL_Reg tty_methods[] = {
  { "getWinSize", tty_get_winsize },
  { "resetMode", tty_reset_mode },
  { "setMode", tty_set_mode },
  { NULL, NULL }
};

static const struct luaL_Reg tty_functions[] = {
  { "new", tty_new },
  { "openFd", tty_open_fd },
  { NULL, NULL }
};

int luaopen_couv_tty(lua_State *L) {
  lua_newtable(L);
  couvL_setfuncs(L, tty_functions, 0);

  couv_newmetatable(L, COUV_TTY_MTBL_NAME, COUV_STREAM_MTBL_NAME);
  couvL_setfuncs(L, tty_methods, 0);
  lua_setmetatable(L, -2);

  lua_setfield(L, -2, "Tty");
  return 0;
}
