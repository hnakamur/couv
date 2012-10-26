#include "couv-private.h"

static int couv_chdir(lua_State *L) {
  const char *dir;
  uv_err_t err;

  dir = luaL_checkstring(L, 1);
  err = uv_chdir(dir);
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  return 0;
}

static int couv_cwd(lua_State *L) {
  char buf[1024];
  uv_err_t err;

  err = uv_cwd(buf, sizeof(buf));
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  lua_pushstring(L, buf);
  return 1;
}

static int couv_exepath(lua_State *L) {
  char buf[1024];
  size_t size;
  int r;

  size = sizeof(buf);
  r = uv_exepath(buf, &size);
  if (r)
    return luaL_error(L, "exepath() failed");
  lua_pushlstring(L, buf, size);
  return 1;
}

static int couv_hrtime(lua_State *L) {
  lua_pushnumber(L, uv_hrtime());
  return 1;
}

static int couv_get_free_memory(lua_State *L) {
  lua_pushnumber(L, uv_get_free_memory());
  return 1;
}

static int couv_get_total_memory(lua_State *L) {
  lua_pushnumber(L, uv_get_total_memory());
  return 1;
}

static int couv_kill(lua_State *L) {
  int pid;
  int signum;
  uv_err_t err;

  pid = luaL_checkint(L, 1);
  signum = luaL_checkint(L, 2);
  err = uv_kill(pid, signum);
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  return 0;
}

static int couv_loadavg(lua_State *L) {
  double avg[3];

  uv_loadavg(avg);
  lua_pushnumber(L, avg[0]);
  lua_pushnumber(L, avg[1]);
  lua_pushnumber(L, avg[2]);
  return 3;
}

static int couv_get_process_title(lua_State *L) {
  char buf[1024];
  uv_err_t err;

  err = uv_get_process_title(buf, sizeof(buf));
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  lua_pushstring(L, buf);
  return 1;
}

static int couv_set_process_title(lua_State *L) {
  const char *title;
  uv_err_t err;

  title = luaL_checkstring(L, 1);
  err = uv_set_process_title(title);
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  return 0;
}

static int couv_resident_set_memory(lua_State *L) {
  size_t rss;
  uv_err_t err;

  rss = luaL_checkinteger(L, 1);
  err = uv_resident_set_memory(&rss);
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  lua_pushnumber(L, rss);
  return 1;
}

static int couv_uptime(lua_State *L) {
  double uptime;
  uv_err_t err;

  err = uv_uptime(&uptime);
  if (err.code != UV_OK)
    return luaL_error(L, couvL_uv_errname(err.code));
  lua_pushnumber(L, uptime);
  return 1;
}

static const struct luaL_Reg functions[] = {
  { "chdir", couv_chdir },
  { "cwd", couv_cwd },
  { "exepath", couv_exepath },
  { "get_free_memory", couv_get_free_memory },
  { "get_process_title", couv_get_process_title },
  { "get_total_memory", couv_get_total_memory },
  { "hrtime", couv_hrtime },
  { "kill", couv_kill },
  { "loadavg", couv_loadavg },
  { "set_process_title", couv_set_process_title },
  { "resident_set_memory", couv_resident_set_memory },
  { "uptime", couv_uptime },
  { NULL, NULL }
};

int luaopen_couv_native(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(functions) - 1);
  couvL_setfuncs(L, functions, 0);

  luaopen_couv_loop(L);

  luaopen_couv_buffer(L);
  luaopen_couv_fs(L);
  luaopen_couv_ipaddr(L);

  /* order superclass to subclasses. */
  luaopen_couv_handle(L);

  /* subclasses of handle. */
  luaopen_couv_process(L);
  luaopen_couv_timer(L);
  luaopen_couv_udp(L);
  luaopen_couv_stream(L);

  /* subclasses of streams. */
  luaopen_couv_pipe(L);
  luaopen_couv_tcp(L);
  luaopen_couv_tty(L);

  return 1;
}
