#include "couv-private.h"

static int tcp_create(lua_State *L) {
  uv_loop_t *loop;
  couv_tcp_t *w_handle;
  int r;

  w_handle = lua_newuserdata(L, sizeof(couv_tcp_t));
  if (!w_handle)
    return 0;

  w_handle->is_yielded_for_read = 0;
  ngx_queue_init(&w_handle->input_queue);
  loop = couv_loop(L);
  r = uv_tcp_init(loop, &w_handle->handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }
  w_handle->handle.data = L;
  return 1;
}

static int tcp_bind(lua_State *L) {
  couv_tcp_t *w_handle;
  struct sockaddr_in *addr;
  int r;

  w_handle = lua_touserdata(L, 1);
  addr = couv_checkip4addr(L, 2);
  r = uv_tcp_bind(&w_handle->handle, *addr);
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
  couv_tcp_t *w_handle;
  struct sockaddr_in *addr;
  uv_connect_t *req;
  int r;

  w_handle  = (couv_tcp_t *)lua_touserdata(L, 1);
  addr = couv_checkip4addr(L, 2);

  req = couv_alloc(L, sizeof(uv_connect_t));
  r = uv_tcp_connect(req, &w_handle->handle, *addr, connect_cb);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}


static const struct luaL_Reg tcp_functions[] = {
  { "tcp_bind", tcp_bind },
  { "tcp_connect", tcp_connect },
  { "tcp_create", tcp_create },
  { NULL, NULL }
};

int luaopen_couv_tcp(lua_State *L) {
  luaL_register(L, NULL, tcp_functions);
  return 1;
}
