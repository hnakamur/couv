#include "couv-private.h"

typedef struct luv_udp_send_s {
  uv_buf_t *bufs;
  uv_udp_send_t req;
} luv_udp_send_t;

static luv_udp_send_t *luv_alloc_udp_send(lua_State *L, uv_buf_t *bufs) {
  luv_udp_send_t *req;

  req = luv_alloc(L, sizeof(luv_udp_send_t));
  if (!req)
    return NULL;

  req->bufs = bufs;
  return req;
}


int luv_udp_create(lua_State *L) {
  uv_loop_t *loop;
  luv_udp_t *lhandle;
  int r;

  lhandle = lua_newuserdata(L, sizeof(luv_udp_t));
  if (!lhandle)
    return 0;

  lhandle->is_yielded_for_recv = 0;
  ngx_queue_init(&lhandle->input_queue);
  loop = luv_loop(L);
  r = uv_udp_init(loop, &lhandle->handle);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  lhandle->handle.data = L;
  return 1;
}

int luv_udp_open(lua_State *L) {
  uv_loop_t *loop;
  luv_udp_t *lhandle;
  uv_os_sock_t sock;
  int r;

  loop = luv_loop(L);
  lhandle = lua_touserdata(L, 1);
  sock = (uv_os_sock_t)luaL_checkinteger(L, 2);
  r = uv_udp_open(&lhandle->handle, sock);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  lhandle->is_yielded_for_recv = 0;
  lhandle->handle.data = L;
  return 0;
}

int luv_udp_bind(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  luv_udp_t *lhandle = lua_touserdata(L, 1);
  struct sockaddr_in *addr = luv_checkip4addr(L, 2);
  int r = uv_udp_bind(&lhandle->handle, *addr, 0);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 0;
}

static void udp_send_cb(uv_udp_send_t* req, int status) {
  luv_udp_send_t *holder;
  uv_loop_t *loop;
  lua_State *L;
  holder = container_of(req, luv_udp_send_t, req);
  loop = req->handle->loop;
  L = req->handle->data;
  luv_free(L, holder->bufs);
  if (status < 0) {
    lua_pushstring(L, luvL_uv_errname(uv_last_error(loop).code));
    luv_resume(L, L, 1);
  } else
    luv_resume(L, L, 0);
}

int luv_udp_send(lua_State *L) {
  int r;
  uv_loop_t *loop;
  luv_udp_t *lhandle;
  struct sockaddr_in *addr;
  uv_buf_t *bufs;
  size_t bufcnt;
  luv_udp_send_t *holder;

  loop = luv_loop(L);
  lhandle = (luv_udp_t *)lua_touserdata(L, 1);
  addr = luv_checkip4addr(L, 2);

  bufs = luv_checkbuforstrtable(L, 3, &bufcnt);
  holder = luv_alloc_udp_send(L, bufs);
  r = uv_udp_send(&holder->req, &lhandle->handle, bufs, (int)bufcnt, *addr,
      udp_send_cb);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return lua_yield(L, 0);
}

static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  lua_State *L;
  void *p;

  L = handle->data;
  p = luv_buf_mem_alloc(L, suggested_size);
  if (!p)
    return uv_buf_init(NULL, 0);
  return uv_buf_init(p, suggested_size);
}

static void udp_recv_cb(uv_udp_t *handle, ssize_t nread, uv_buf_t buf,
    struct sockaddr* addr, unsigned flags) {
  luv_udp_t *lhandle;
  lua_State *L;
  luv_udp_input_t *input;

  lhandle = container_of(handle, luv_udp_t, handle);
  L = handle->data;

  input = luv_alloc(L, sizeof(luv_udp_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->lbuf.orig = buf.base;
  input->lbuf.buf = buf;
  if (addr)
    input->addr.v4 = *(struct sockaddr_in *)addr;
  ngx_queue_insert_tail(&lhandle->input_queue, (ngx_queue_t *)input);

  if (lua_status(L) == LUA_YIELD && lhandle->is_yielded_for_recv) {
    lhandle->is_yielded_for_recv = 0;
    luv_resume(L, L, 0);
  }
}

int luv_udp_recv_start(lua_State *L) {
  luv_udp_t *lhandle;
  int r;

  lhandle = lua_touserdata(L, 1);
  r = uv_udp_recv_start(&lhandle->handle, alloc_cb, udp_recv_cb);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

int luv_udp_recv_stop(lua_State *L) {
  luv_udp_t *lhandle;
  int r;

  lhandle = lua_touserdata(L, 1);
  lhandle->handle.data = L;
  r = uv_udp_recv_stop(&lhandle->handle);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

int luv_udp_prim_recv(lua_State *L) {
  luv_udp_t *lhandle;
  luv_udp_input_t *input;
  luv_buf_t *lbuf;
  struct sockaddr_in *ip4addr;

  lhandle = lua_touserdata(L, 1);
  if (ngx_queue_empty(&lhandle->input_queue)) {
    lhandle->is_yielded_for_recv = 1;
    return lua_yield(L, 0);
  }
  input = (luv_udp_input_t *)ngx_queue_head(&lhandle->input_queue);
  ngx_queue_remove(input);

  lua_pushnumber(L, input->nread);

  lbuf = lua_newuserdata(L, sizeof(luv_buf_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  *lbuf = input->lbuf;

  ip4addr = lua_newuserdata(L, sizeof(struct sockaddr_in));
  luaL_getmetatable(L, LUV_IP4ADDR_MTBL_NAME);
  lua_setmetatable(L, -2);
  *ip4addr = input->addr.v4;

  return 3;
}



static const struct luaL_Reg udp_functions[] = {
  { "udp_bind", luv_udp_bind },
  { "udp_create", luv_udp_create },
  { "udp_open", luv_udp_open },
  { "udp_prim_recv", luv_udp_prim_recv },
  { "udp_recv_start", luv_udp_recv_start },
  { "udp_recv_stop", luv_udp_recv_stop },
  { "udp_send", luv_udp_send },
  { NULL, NULL }
};

int luaopen_couv_udp(lua_State *L) {
  luaL_register(L, NULL, udp_functions);

  return 1;
}
