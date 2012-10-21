#include "couv-private.h"

static uv_tcp_t *couv_alloc_tcp_handle(lua_State *L) {
  couv_tcp_t *w_handle;

  w_handle = couv_alloc(L, sizeof(couv_tcp_t));
  if (!w_handle)
    return NULL;

  if (couvL_is_mainthread(L)) {
    luaL_error(L, "tcp handle must be created in coroutine, not in main thread.");
    return NULL;
  } else
    w_handle->threadref = luaL_ref(L, LUA_REGISTRYINDEX);
  return &w_handle->handle;
}

void couv_free_tcp_handle(lua_State *L, uv_tcp_t *handle) {
  couv_tcp_t *w_handle;

  w_handle = container_of(handle, couv_tcp_t, handle);
  luaL_unref(L, LUA_REGISTRYINDEX, w_handle->threadref);
  couv_free(L, w_handle);
}

static int tcp_create(lua_State *L) {
  uv_loop_t *loop;
  uv_tcp_t *handle;
  couv_tcp_t *w_handle;
  int r;

  handle = couv_alloc_tcp_handle(L);
  if (!handle)
    return 0;

  w_handle = container_of(handle, couv_tcp_t, handle);
  w_handle->is_yielded_for_read = 0;
  ngx_queue_init(&w_handle->input_queue);
  loop = couv_loop(L);
  r = uv_tcp_init(loop, handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }
  handle->data = L;
  lua_pushlightuserdata(L, handle);
  return 1;
}

static int tcp_open(lua_State *L) {
  uv_tcp_t *handle;
  couv_tcp_t *w_handle;
  uv_os_sock_t sock;
  int r;

  handle = lua_touserdata(L, 1);
  sock = (uv_os_sock_t)luaL_checkinteger(L, 2);
  r = uv_tcp_open(handle, sock);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  w_handle = container_of(handle, couv_tcp_t, handle);
  w_handle->is_yielded_for_read = 0;
  return 0;
}

static int tcp_bind(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr_in *addr;
  int r;

  handle = lua_touserdata(L, 1);
  addr = couv_checkip4addr(L, 2);
  r = uv_tcp_bind(handle, *addr);
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
  struct sockaddr_in *addr;
  uv_connect_t *req;
  int r;

  handle = lua_touserdata(L, 1);
  addr = couv_checkip4addr(L, 2);

  req = couv_alloc(L, sizeof(uv_connect_t));
  r = uv_tcp_connect(req, handle, *addr, connect_cb);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}

static int tcp_nodelay(lua_State *L) {
  uv_tcp_t *handle;
  int enable;
  int r;

  handle = lua_touserdata(L, 1);
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

  handle = lua_touserdata(L, 1);
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

  handle = lua_touserdata(L, 1);
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

  handle = lua_touserdata(L, 1);
  namelen = sizeof(name);
  r = uv_tcp_getsockname(handle, (struct sockaddr *)&name, &namelen);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return couv_push_ipaddr_raw(L, (struct sockaddr *)&name);
}

static int tcp_getpeername(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr_storage name;
  int namelen;
  int r;

  handle = lua_touserdata(L, 1);
  namelen = sizeof(name);
  r = uv_tcp_getpeername(handle, (struct sockaddr *)&name, &namelen);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return couv_push_ipaddr_raw(L, (struct sockaddr *)&name);
}

static const struct luaL_Reg tcp_functions[] = {
  { "tcp_bind", tcp_bind },
  { "tcp_connect", tcp_connect },
  { "tcp_create", tcp_create },
  { "tcp_open", tcp_open },
  { "tcp_keepalive", tcp_keepalive },
  { "tcp_nodelay", tcp_nodelay },
  { "tcp_getpeername", tcp_getpeername },
  { "tcp_simultaneous_accepts", tcp_simultaneous_accepts },
  { "tcp_getsockname", tcp_getsockname },
  { NULL, NULL }
};

int luaopen_couv_tcp(lua_State *L) {
  couvL_setfuncs(L, tcp_functions, 0);
  return 1;
}
