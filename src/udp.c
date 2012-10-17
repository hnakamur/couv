#include "couv-private.h"

typedef struct couv_udp_send_s {
  uv_buf_t *bufs;
  uv_udp_send_t req;
} couv_udp_send_t;

static couv_udp_send_t *couv_alloc_udp_send(lua_State *L, uv_buf_t *bufs) {
  couv_udp_send_t *req;

  req = couv_alloc(L, sizeof(couv_udp_send_t));
  if (!req)
    return NULL;

  req->bufs = bufs;
  return req;
}


static uv_udp_t *couv_alloc_udp_handle(lua_State *L) {
  couv_udp_t *w_handle;

  w_handle = couv_alloc(L, sizeof(couv_udp_t));
  if (!w_handle)
    return NULL;

  if (couvL_is_mainthread(L)) {
    luaL_error(L, "udp handle must be created in coroutine, not in main thread.");
    return NULL;
  } else
    w_handle->threadref = luaL_ref(L, LUA_REGISTRYINDEX);
  return &w_handle->handle;
}

void couv_free_udp_handle(lua_State *L, uv_udp_t *handle) {
  couv_udp_t *w_handle;

  w_handle = container_of(handle, couv_udp_t, handle);
  luaL_unref(L, LUA_REGISTRYINDEX, w_handle->threadref);
  couv_free(L, w_handle);
}

int couv_udp_create(lua_State *L) {
  uv_loop_t *loop;
  uv_udp_t *handle;
  couv_udp_t *w_handle;
  int r;

  handle = couv_alloc_udp_handle(L);
  if (!handle)
    return 0;

  loop = couv_loop(L);
  r = uv_udp_init(loop, handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }

  handle->data = L;
  w_handle = container_of(handle, couv_udp_t, handle);
  w_handle->is_yielded_for_recv = 0;
  ngx_queue_init(&w_handle->input_queue);

  lua_pushlightuserdata(L, handle);
  return 1;
}

int couv_udp_open(lua_State *L) {
  uv_udp_t *handle;
  couv_udp_t *w_handle;
  uv_os_sock_t sock;
  int r;

  handle = lua_touserdata(L, 1);
  sock = (uv_os_sock_t)luaL_checkinteger(L, 2);
  r = uv_udp_open(handle, sock);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  w_handle = container_of(handle, couv_udp_t, handle);
  w_handle->is_yielded_for_recv = 0;
  return 0;
}

int couv_udp_bind(lua_State *L) {
  uv_udp_t *handle;
  struct sockaddr_in *addr;
  int r;

  handle = lua_touserdata(L, 1);
  addr = couv_checkip4addr(L, 2);
  r = uv_udp_bind(handle, *addr, 0);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static void udp_send_cb(uv_udp_send_t* req, int status) {
  couv_udp_send_t *holder;
  lua_State *L;
  int nresults;

  holder = container_of(req, couv_udp_send_t, req);
  L = req->handle->data;
  if (status < 0) {
    lua_pushstring(L, couvL_uv_errname(uv_last_error(req->handle->loop).code));
    nresults = 1;
  } else
    nresults = 0;
  couv_free(L, holder->bufs);
  couv_free(L, holder);
  couv_resume(L, L, nresults);
}

int couv_udp_send(lua_State *L) {
  uv_udp_t *handle;
  struct sockaddr_in *addr;
  uv_buf_t *bufs;
  size_t bufcnt;
  couv_udp_send_t *holder;
  int r;

  handle = lua_touserdata(L, 1);
  addr = couv_checkip4addr(L, 2);
  bufs = couv_checkbuforstrtable(L, 3, &bufcnt);
  holder = couv_alloc_udp_send(L, bufs);
  r = uv_udp_send(&holder->req, handle, bufs, (int)bufcnt, *addr, udp_send_cb);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}

static void udp_recv_cb(uv_udp_t *handle, ssize_t nread, uv_buf_t buf,
    struct sockaddr* addr, unsigned flags) {
  couv_udp_t *w_handle;
  lua_State *L;
  couv_udp_input_t *input;

  w_handle = container_of(handle, couv_udp_t, handle);
  L = handle->data;

  input = couv_alloc(L, sizeof(couv_udp_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->w_buf.orig = buf.base;
  input->w_buf.buf = buf;
  if (addr)
    input->addr.v4 = *(struct sockaddr_in *)addr;
  ngx_queue_insert_tail(&w_handle->input_queue, (ngx_queue_t *)input);

  if (lua_status(L) == LUA_YIELD && w_handle->is_yielded_for_recv) {
    w_handle->is_yielded_for_recv = 0;
    couv_resume(L, L, 0);
  }
}

int couv_udp_recv_start(lua_State *L) {
  uv_udp_t *handle;
  int r;

  handle = lua_touserdata(L, 1);
  r = uv_udp_recv_start(handle, couv_buf_alloc_cb, udp_recv_cb);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

int couv_udp_recv_stop(lua_State *L) {
  uv_udp_t *handle;
  int r;

  handle = lua_touserdata(L, 1);
  r = uv_udp_recv_stop(handle);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

int couv_udp_prim_recv(lua_State *L) {
  uv_udp_t *handle;
  couv_udp_t *w_handle;
  couv_udp_input_t *input;
  couv_buf_t *w_buf;
  struct sockaddr_in *ip4addr;

  handle = lua_touserdata(L, 1);
  w_handle = container_of(handle, couv_udp_t, handle);
  if (ngx_queue_empty(&w_handle->input_queue)) {
    w_handle->is_yielded_for_recv = 1;
    return lua_yield(L, 0);
  }
  input = (couv_udp_input_t *)ngx_queue_head(&w_handle->input_queue);
  ngx_queue_remove(input);

  lua_pushnumber(L, input->nread);

  w_buf = lua_newuserdata(L, sizeof(couv_buf_t));
  luaL_getmetatable(L, COUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  *w_buf = input->w_buf;

  ip4addr = lua_newuserdata(L, sizeof(struct sockaddr_in));
  luaL_getmetatable(L, COUV_IP4ADDR_MTBL_NAME);
  lua_setmetatable(L, -2);
  *ip4addr = input->addr.v4;

  return 3;
}



static const struct luaL_Reg udp_functions[] = {
  { "udp_bind", couv_udp_bind },
  { "udp_create", couv_udp_create },
  { "udp_open", couv_udp_open },
  { "udp_prim_recv", couv_udp_prim_recv },
  { "udp_recv_start", couv_udp_recv_start },
  { "udp_recv_stop", couv_udp_recv_stop },
  { "udp_send", couv_udp_send },
  { NULL, NULL }
};

int luaopen_couv_udp(lua_State *L) {
  luaL_register(L, NULL, udp_functions);

  return 1;
}
