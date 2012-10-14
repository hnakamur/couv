#include "yaluv-private.h"

typedef struct luv_udp_send_s {
  lua_State *L;
  uv_buf_t *bufs;
  uv_udp_send_t req;
} luv_udp_send_t;

static luv_udp_send_t *luv_alloc_udp_send(lua_State *L, uv_buf_t *bufs) {
  luv_udp_send_t *req;

  req = luv_alloc(L, sizeof(luv_udp_send_t));
  if (!req)
    return NULL;

  req->L = L;
  req->bufs = bufs;
  return req;
}


int luv_udp_create(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  uv_udp_t *handle = lua_newuserdata(L, sizeof(uv_udp_t));
  int r = uv_udp_init(loop, handle);
printf("create L=%lx, handle=%lx, r=%d\n", (unsigned long)L, (unsigned long)handle, r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 1;
}

int luv_udp_open(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  uv_udp_t *handle = (uv_udp_t *)lua_touserdata(L, 1);
  uv_os_sock_t sock = (uv_os_sock_t)luaL_checkinteger(L, 2);
  int r = uv_udp_open(handle, sock);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 0;
}

int luv_udp_bind(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  uv_udp_t *handle = (uv_udp_t *)lua_touserdata(L, 1);
  struct sockaddr_in *addr = luv_checkip4addr(L, 2);
  int r = uv_udp_bind(handle, *addr, 0);
printf("bind L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)handle);
ip4addr_dbg_print("bind", addr);
printf("bind r=%d\n", r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 0;
}

static void udp_send_cb(uv_udp_send_t* req, int status) {
  luv_udp_send_t *holder;
  uv_loop_t *loop;
  lua_State *L;
printf("udp_send_cb status=%d\n", status);
  holder = container_of(req, luv_udp_send_t, req);
  L = holder->L;
  loop = req->handle->loop;
printf("udp_send_cb loop=%lx, L=%lx\n", (unsigned long)loop, (unsigned long)L);
#if 0
  free(req->data);
#endif
  if (status < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  luv_resume(L, L, 0);
}

int luv_udp_send(lua_State *L) {
  int r;
  uv_loop_t *loop;
  uv_udp_t *handle;
  struct sockaddr_in *addr;
  uv_buf_t *bufs;
  size_t bufcnt;
  luv_udp_send_t *holder;

  loop = luv_loop(L);
  handle = (uv_udp_t *)lua_touserdata(L, 1);
  addr = luv_checkip4addr(L, 2);

  bufs = luv_checkbuforstrtable(L, 3, &bufcnt);
  holder = luv_alloc_udp_send(L, bufs);
  r = uv_udp_send(&holder->req, handle, bufs, (int)bufcnt, *addr, udp_send_cb);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return lua_yield(L, 0);
}

static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  lua_State *L;
  void *p;

  L = (lua_State *)handle->data;
  p = luv_alloc(L, suggested_size);
  if (!p)
    return uv_buf_init(NULL, 0);
  return uv_buf_init(p, suggested_size);
}

static void udp_recv_cb(uv_udp_t* handle, ssize_t nread, uv_buf_t buf,
    struct sockaddr* addr, unsigned flags) {
  uv_loop_t *loop;
  lua_State *L;
  luv_buf_t *lbuf;
  struct sockaddr_in *ip4addr;
  int r;

  loop = handle->loop;
  L = (lua_State *)handle->data;
printf("recv_cb #1 L=%lx, top=%d\n", (unsigned long)L, lua_gettop(L));

printf("recv_cb #2 handle=%lx, nread=%ld, buf.base=%s\n", (unsigned long)handle, nread, buf.base);
  r = uv_udp_recv_stop(handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
    return;
  }

  if (nread == 0) {
    lua_pushnil(L);
    luv_resume(L, L, 1);
    return;
  }

  lua_pushnumber(L, nread);

  lbuf = lua_newuserdata(L, sizeof(luv_buf_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  lbuf->orig = buf.base;
  lbuf->buf = buf;

  ip4addr = lua_newuserdata(L, sizeof(struct sockaddr_in));
  luaL_getmetatable(L, LUV_IP4ADDR_MTBL_NAME);
  lua_setmetatable(L, -2);
  *ip4addr = *(struct sockaddr_in *)addr;

  luv_resume(L, L, 3);
}

int luv_udp_recv(lua_State *L) {
  int r;
  uv_loop_t *loop;
  uv_udp_t *handle;

  loop = luv_loop(L);
  handle = (uv_udp_t *)lua_touserdata(L, 1);
  handle->data = L;
printf("udp_recv #3 handle=%lx, L=%lx, top=%d\n", (unsigned long)handle, (unsigned long)L, lua_gettop(L));
  r = uv_udp_recv_start(handle, alloc_cb, udp_recv_cb);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return lua_yield(L, 0);
}


#if 0
/*
 * Bind to a IPv6 address and port.
 *
 * Arguments:
 *  handle    UDP handle. Should have been initialized with `uv_udp_init`.
 *  addr      struct sockaddr_in with the address and port to bind to.
 *  flags     Should be 0 or UV_UDP_IPV6ONLY.
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_bind6(uv_udp_t* handle, struct sockaddr_in6 addr,
    unsigned flags);
UV_EXTERN int uv_udp_getsockname(uv_udp_t* handle, struct sockaddr* name,
    int* namelen);

/*
 * Set membership for a multicast address
 *
 * Arguments:
 *  handle              UDP handle. Should have been initialized with
 *                      `uv_udp_init`.
 *  multicast_addr      multicast address to set membership for
 *  interface_addr      interface address
 *  membership          Should be UV_JOIN_GROUP or UV_LEAVE_GROUP
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_set_membership(uv_udp_t* handle,
    const char* multicast_addr, const char* interface_addr,
    uv_membership membership);

/*
 * Set IP multicast loop flag. Makes multicast packets loop back to
 * local sockets.
 *
 * Arguments:
 *  handle              UDP handle. Should have been initialized with
 *                      `uv_udp_init`.
 *  on                  1 for on, 0 for off
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_set_multicast_loop(uv_udp_t* handle, int on);

/*
 * Set the multicast ttl
 *
 * Arguments:
 *  handle              UDP handle. Should have been initialized with
 *                      `uv_udp_init`.
 *  ttl                 1 through 255
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_set_multicast_ttl(uv_udp_t* handle, int ttl);

/*
 * Set broadcast on or off
 *
 * Arguments:
 *  handle              UDP handle. Should have been initialized with
 *                      `uv_udp_init`.
 *  on                  1 for on, 0 for off
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_set_broadcast(uv_udp_t* handle, int on);

/*
 * Set the time to live
 *
 * Arguments:
 *  handle              UDP handle. Should have been initialized with
 *                      `uv_udp_init`.
 *  ttl                 1 through 255
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_set_ttl(uv_udp_t* handle, int ttl);

/*
 * Send data. If the socket has not previously been bound with `uv_udp_bind`
 * or `uv_udp_bind6`, it is bound to 0.0.0.0 (the "all interfaces" address)
 * and a random port number.
 *
 * Arguments:
 *  req       UDP request handle. Need not be initialized.
 *  handle    UDP handle. Should have been initialized with `uv_udp_init`.
 *  bufs      List of buffers to send.
 *  bufcnt    Number of buffers in `bufs`.
 *  addr      Address of the remote peer. See `uv_ip4_addr`.
 *  send_cb   Callback to invoke when the data has been sent out.
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_send(uv_udp_send_t* req, uv_udp_t* handle,
    uv_buf_t bufs[], int bufcnt, struct sockaddr_in addr,
    uv_udp_send_cb send_cb);

/*
 * Send data. If the socket has not previously been bound with `uv_udp_bind6`,
 * it is bound to ::0 (the "all interfaces" address) and a random port number.
 *
 * Arguments:
 *  req       UDP request handle. Need not be initialized.
 *  handle    UDP handle. Should have been initialized with `uv_udp_init`.
 *  bufs      List of buffers to send.
 *  bufcnt    Number of buffers in `bufs`.
 *  addr      Address of the remote peer. See `uv_ip6_addr`.
 *  send_cb   Callback to invoke when the data has been sent out.
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_send6(uv_udp_send_t* req, uv_udp_t* handle,
    uv_buf_t bufs[], int bufcnt, struct sockaddr_in6 addr,
    uv_udp_send_cb send_cb);
#endif

static const struct luaL_Reg udp_functions[] = {
  { "udp_bind", luv_udp_bind },
  { "udp_create", luv_udp_create },
  { "udp_open", luv_udp_open },
  { "udp_recv", luv_udp_recv },
  { "udp_send", luv_udp_send },
  { NULL, NULL }
};

int luaopen_yaluv_udp(lua_State *L) {
  luaL_register(L, NULL, udp_functions);

  return 1;
}
