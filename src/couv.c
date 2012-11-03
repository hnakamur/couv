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

static void getaddrinfo_cb(uv_getaddrinfo_t *req, int status,
    struct addrinfo *res) {
  lua_State *L;
  struct addrinfo *p;
  int i;
  int nargs;

  L = req->data;

  if (status == 0) {
    lua_newtable(L);
    for (p = res, i = 1; p; p = p->ai_next, ++i) {
      lua_newtable(L);

      couvL_SET_FIELD(L, family, number, p->ai_family);
      couvL_SET_FIELD(L, socktype, number, p->ai_socktype);
      couvL_SET_FIELD(L, protocol, number, p->ai_protocol);

      couvL_pushsockaddr(L, p->ai_addr);
      lua_setfield(L, -2, "addr");

      lua_rawseti(L, -2, i);
    }
    uv_freeaddrinfo(res);
    nargs = 1;
  } else {
    lua_pushnil(L);
    lua_pushstring(L, couvL_uv_lasterrname(req->loop));
    nargs = 2;
  }
  couv_free(L, req);
  couv_resume(L, L, nargs);
}

static struct addrinfo *couvL_checkaddrinfohints(lua_State *L, int index,
    struct addrinfo *hints) {
  int hints_type;
  struct addrinfo *ret;

  ret = NULL;
  hints_type = lua_type(L, index);
  if (hints_type == LUA_TTABLE) {
    lua_getfield(L, index, "family");
    if (!lua_isnil(L, -1))
      ret = hints;
    hints->ai_family = couvL_tosockfamily(L, -1);
    luaL_argcheck(L, hints->ai_family != -1, index,
        "value at \"family\" key must be AF_UNSPEC, AF_INET or AF_INET6");

    lua_getfield(L, index, "socktype");
    if (!lua_isnil(L, -1))
      ret = hints;
    hints->ai_socktype = lua_tointeger(L, -1);

    lua_getfield(L, index, "protocol");
    if (!lua_isnil(L, -1))
      ret = hints;
    hints->ai_protocol = lua_tointeger(L, -1);

    lua_getfield(L, index, "flags");
    if (!lua_isnil(L, -1))
      ret = hints;
    hints->ai_flags = lua_tointeger(L, -1);
  } else
    luaL_argcheck(L, hints_type == LUA_TNIL || hints_type == LUA_TNONE, index,
        "must be table or nil");
  return ret;
}

static int couv_getaddrinfo(lua_State *L) {
  const char *node;
  const char *service;
  struct addrinfo hints;
  struct addrinfo *hints_ptr;
  uv_loop_t *loop;
  uv_getaddrinfo_t *req;
  int r;

  node = luaL_optstring(L, 1, NULL);
  service = luaL_optstring(L, 2, NULL);
  luaL_argcheck(L, node || service, 1, "node or service must not be nil");
  memset(&hints, 0, sizeof(hints));
  hints_ptr = couvL_checkaddrinfohints(L, 3, &hints);
  if (couvL_is_mainthread(L))
    luaL_error(L, "getaddrinfo must be called in coroutine.");
  req = couv_alloc(L, sizeof(uv_getaddrinfo_t));
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(req));
  req->data = L;
  loop = couv_loop(L);
  r = uv_getaddrinfo(loop, req, getaddrinfo_cb, node, service, hints_ptr);
  if (r < 0) {
    couv_free(L, req);
    luaL_error(L, couvL_uv_lasterrname(loop));
  }
  return lua_yield(L, 0);
}

static void sleep_close_cb(uv_handle_t *handle) {
  lua_State *L;

  L = handle->data;

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));

  couv_free(L, handle);
  couv_resume(L, L, 0);
}

static void sleep_cb(uv_timer_t *handle, int status) {
  uv_close((uv_handle_t *)handle, sleep_close_cb);
}

static int couv_sleep(lua_State *L) {
  uv_loop_t *loop;
  uv_timer_t *handle;
  int64_t timeout;
  int r;

  timeout = luaL_checkinteger(L, 1);
  if (couvL_is_mainthread(L))
    luaL_error(L, "sleep must be called in coroutine.");
  handle = couv_alloc(L, sizeof(uv_timer_t));
  loop = couv_loop(L);
  uv_timer_init(loop, handle);
  handle->data = L;
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));
  r = uv_timer_start(handle, sleep_cb, timeout, 0);
  if (r < 0)
    return luaL_error(L, couvL_uv_lasterrname(loop));
  return lua_yield(L, 0);
}

static const struct luaL_Reg functions[] = {
  { "chdir", couv_chdir },
  { "cwd", couv_cwd },
  { "exepath", couv_exepath },
  { "getaddrinfo", couv_getaddrinfo },
  { "getFreeMemory", couv_get_free_memory },
  { "getProcessTitle", couv_get_process_title },
  { "getTotalMemory", couv_get_total_memory },
  { "hrtime", couv_hrtime },
  { "kill", couv_kill },
  { "loadavg", couv_loadavg },
  { "setProcessTitle", couv_set_process_title },
  { "residentSetMemory", couv_resident_set_memory },
  { "sleep", couv_sleep },
  { "uptime", couv_uptime },
  { NULL, NULL }
};

int luaopen_couv_native(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(functions) - 1);
  couvL_setfuncs(L, functions, 0);

  luaopen_couv_loop(L);

  luaopen_couv_buffer(L);
  luaopen_couv_fs(L);
  luaopen_couv_sockaddr(L);

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
