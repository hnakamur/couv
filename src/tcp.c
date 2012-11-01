#include "couv-private.h"

static uv_tcp_t *couv_new_tcp_handle(lua_State *L) {
  couv_tcp_t *w_handle;
  uv_tcp_t *handle;

  w_handle = lua_newuserdata(L, sizeof(couv_tcp_t));
  if (!w_handle)
    return NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, COUV_TCP_MTBL_NAME);
  lua_setmetatable(L, -2);

  handle = &w_handle->handle;

  if (couvL_is_mainthread(L)) {
    luaL_error(L, "tcp handle must be created in coroutine, not in main thread.");
    return NULL;
  } else
    couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));
  return handle;
}

void couv_clean_tcp_handle(lua_State *L, uv_tcp_t *handle) {
  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_LISTEN_CB_REG_KEY(handle));
}

static int tcp_new(lua_State *L) {
  uv_loop_t *loop;
  uv_tcp_t *handle;
  couv_stream_handle_data_t *hdata;
  int r;

  handle = couv_new_tcp_handle(L);
  if (!handle)
    return 0;

  loop = couv_loop(L);
  r = uv_tcp_init(loop, handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }

  handle->data = L;
  hdata = couv_get_stream_handle_data((uv_stream_t *)handle);
  ngx_queue_init(&hdata->input_queue);

  lua_pushvalue(L, -1);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
  return 1;
}

static int tcp_open(lua_State *L) {
  uv_tcp_t *handle;
  uv_os_sock_t sock;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  sock = (uv_os_sock_t)luaL_checkinteger(L, 2);
  r = uv_tcp_open(handle, sock);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }

  handle->data = L;
  return 0;
}

static int tcp_bind(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr *addr;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  addr = couvL_checkudataclass(L, 2, COUV_SOCK_ADDR_MTBL_NAME);
  if (addr->sa_family == AF_INET)
    r = uv_tcp_bind(handle, *(struct sockaddr_in *)addr);
  else
    r = uv_tcp_bind6(handle, *(struct sockaddr_in6 *)addr);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static void connect_cb(uv_connect_t *req, int status) {
  lua_State *L;
  int nresults = 0;

  L = req->handle->data;
  if (status < 0) {
    lua_pushstring(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    nresults = 1;
  }

  couv_free(L, req);
  couv_resume(L, L, nresults);
}

static int tcp_connect(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr *addr;
  uv_connect_t *req;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  addr = couvL_checkudataclass(L, 2, COUV_SOCK_ADDR_MTBL_NAME);
  req = couv_alloc(L, sizeof(uv_connect_t));
  if (addr->sa_family == AF_INET)
    r = uv_tcp_connect(req, handle, *(struct sockaddr_in *)addr, connect_cb);
  else
    r = uv_tcp_connect6(req, handle, *(struct sockaddr_in6 *)addr, connect_cb);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}

static int tcp_nodelay(lua_State *L) {
  uv_tcp_t *handle;
  int enable;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  enable = lua_toboolean(L, 2);
  r = uv_tcp_nodelay(handle, enable);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int tcp_keepalive(lua_State *L) {
  uv_tcp_t *handle;
  int enable;
  unsigned int delay;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  enable = lua_toboolean(L, 2);
  delay = luaL_optinteger(L, 3, 0);
  r = uv_tcp_keepalive(handle, enable, delay);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int tcp_simultaneous_accepts(lua_State *L) {
  uv_tcp_t *handle;
  int enable;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  enable = lua_toboolean(L, 2);
  r = uv_tcp_simultaneous_accepts(handle, enable);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int tcp_getsockname(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr_storage name;
  int namelen;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  namelen = sizeof(name);
  r = uv_tcp_getsockname(handle, (struct sockaddr *)&name, &namelen);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return couv_sockaddr_push_raw(L, (struct sockaddr *)&name);
}

static int tcp_getpeername(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr_storage name;
  int namelen;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_TCP_MTBL_NAME);
  namelen = sizeof(name);
  r = uv_tcp_getpeername(handle, (struct sockaddr *)&name, &namelen);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return couv_sockaddr_push_raw(L, (struct sockaddr *)&name);
}

static const struct luaL_Reg tcp_methods[] = {
  { "bind", tcp_bind },
  { "_connect", tcp_connect },
  { "open", tcp_open },
  { "keepalive", tcp_keepalive },
  { "nodelay", tcp_nodelay },
  { "getpeername", tcp_getpeername },
  { "simultaneousAccepts", tcp_simultaneous_accepts },
  { "getsockname", tcp_getsockname },
  { NULL, NULL }
};

static const struct luaL_Reg tcp_functions[] = {
  { "new", tcp_new },
  { NULL, NULL }
};

int luaopen_couv_tcp(lua_State *L) {
  lua_newtable(L);
  couvL_setfuncs(L, tcp_functions, 0);
  lua_setfield(L, -2, "Tcp");

  couv_newmetatable(L, COUV_TCP_MTBL_NAME, COUV_STREAM_MTBL_NAME);
  couvL_setfuncs(L, tcp_methods, 0);
  lua_setfield(L, -2, "_Tcp");

  return 0;
}
