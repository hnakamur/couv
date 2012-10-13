#include "yaluv-private.h"

static int tcp_create(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  uv_tcp_t *handle = lua_newuserdata(L, sizeof(uv_tcp_t));
  int r = uv_tcp_init(loop, handle);
printf("tcp_create L=%lx, handle=%lx, r=%d\n", (unsigned long)L, (unsigned long)handle, r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 1;
}

static int tcp_bind(lua_State *L) {
  uv_tcp_t *handle = (uv_tcp_t *)lua_touserdata(L, 1);
  struct sockaddr_in *addr = luv_checkip4addr(L, 2);
  int r = uv_tcp_bind(handle, *addr);
printf("tcp_bind L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)handle);
ip4addr_dbg_print("tcp_bind", addr);
printf("tcp_bind r=%d\n", r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

static void connect_cb(uv_connect_t *req, int status) {
  lua_State *L;

  L = req->data;
  if (status < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
    return;
  }

  luv_free(L, req);
  luv_resume(L, L, 0);
}

static int tcp_connect(lua_State *L) {
  uv_tcp_t *handle;
  struct sockaddr_in *addr;
  uv_connect_t *req;
  int r;

  handle  = (uv_tcp_t *)lua_touserdata(L, 1);
  addr = luv_checkip4addr(L, 2);

  req = luv_alloc(L, sizeof(uv_connect_t));
  req->data = L;

  r = uv_tcp_connect(req, handle, *addr, connect_cb);
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
