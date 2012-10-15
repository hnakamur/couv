#include "yaluv-private.h"

static int tcp_create(lua_State *L) {
  uv_loop_t *loop;
  luv_tcp_t *lhandle;
  int r;

  lhandle = lua_newuserdata(L, sizeof(luv_tcp_t));
  if (!lhandle)
    return 0;

  lhandle->is_yielded_for_read = 0;
  ngx_queue_init(&lhandle->input_queue);
  loop = luv_loop(L);
  r = uv_tcp_init(loop, &lhandle->handle);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  lhandle->handle.data = L;
  return 1;
}

static int tcp_bind(lua_State *L) {
  luv_tcp_t *lhandle;
  struct sockaddr_in *addr;
  int r;

  lhandle = lua_touserdata(L, 1);
  addr = luv_checkip4addr(L, 2);
  r = uv_tcp_bind(&lhandle->handle, *addr);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

static void connect_cb(uv_connect_t *req, int status) {
  lua_State *L;
  int nresults = 0;

  L = req->handle->data;
  if (status < 0) {
    lua_pushstring(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
    nresults = 1;
  }

  luv_free(L, req);
  luv_resume(L, L, nresults);
}

static int tcp_connect(lua_State *L) {
  luv_tcp_t *lhandle;
  struct sockaddr_in *addr;
  uv_connect_t *req;
  int r;

  lhandle  = (luv_tcp_t *)lua_touserdata(L, 1);
  addr = luv_checkip4addr(L, 2);

  req = luv_alloc(L, sizeof(uv_connect_t));
  r = uv_tcp_connect(req, &lhandle->handle, *addr, connect_cb);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return lua_yield(L, 0);
}


static const struct luaL_Reg tcp_functions[] = {
  { "tcp_bind", tcp_bind },
  { "tcp_connect", tcp_connect },
  { "tcp_create", tcp_create },
  { NULL, NULL }
};

int luaopen_yaluv_tcp(lua_State *L) {
  luaL_register(L, NULL, tcp_functions);
  return 1;
}
