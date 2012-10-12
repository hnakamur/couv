#include <lauxlib.h>
#include <stdlib.h>
#include <uv.h>
#include "auxlib.h"
#include "buffer.h"
#include "ipaddr.h"
#include "loop.h"
#include "udp.h"

int luv_udp_create(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  uv_udp_t *handle = lua_newuserdata(L, sizeof(uv_udp_t));
  int r = uv_udp_init(loop, handle);
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
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 0;
}

static void udp_send_cb(uv_udp_send_t* req, int status) {
  uv_loop_t *loop = req->handle->loop;
  lua_State *L = (lua_State *)loop->data;
  free(req->data);
  if (status < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  lua_resume(L, 0);
}

int luv_udp_send(lua_State *L) {
  int r;
  uv_loop_t *loop;
  uv_udp_t *handle;
  struct sockaddr_in *addr;
  size_t bufcnt;
  char buf[sizeof "255.255.255.255"];
  unsigned char *p;
  int i;

printf("udp_send #1\n");
  loop = luv_loop(L);
  handle = (uv_udp_t *)lua_touserdata(L, 1);
  addr = luv_checkip4addr(L, 2);
  r = uv_ip4_name(addr, buf, sizeof(buf));
  p = (unsigned char *)&addr->sin_port;
printf("udb_send host=%s, port=%d\n", buf,  p[0] << 8 | p[1]);

  uv_buf_t *bufs = luv_checkbuforstrtable(L, 3, &bufcnt);
printf("udp_send #1.1 bufcnt=%d\n", bufcnt);
for (i = 0; i < bufcnt; ++i) {
printf("i=%d, buf.len=%d, buf.base=%s\n", i, bufs[i].len, bufs[i].base);
}
  uv_udp_send_t* req = malloc(sizeof(uv_udp_send_t));
  req->data = bufs;
printf("udp_send #2\n");
  r = uv_udp_send(req, handle, bufs, (int)bufcnt, *addr, udp_send_cb);
printf("udp_send #3 r=%d\n", r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
printf("udp_send #4\n");
  return lua_yield(L, 0);
}

static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  uv_loop_t *loop;
  lua_State *L;
  uv_buf_t buf;
printf("alloc_cb #1 suggested_size=%u\n", suggested_size);
  loop = handle->loop;
  L = (lua_State *)loop->data;
printf("alloc_cb #2 loop=%x, L=%x\n", (unsigned long)loop, (unsigned long)L);
#if 0
  buf.base = (char *)lua_newuserdata(L, suggested_size);
  buf.len = suggested_size;
  lua_pop(L, 1);
#endif
  buf.base = (char *)malloc(suggested_size);
  buf.len = suggested_size;
printf("alloc_cb #3\n");
  return buf;
}

static void udp_recv_cb(uv_udp_t* handle, ssize_t nread, uv_buf_t buf,
    struct sockaddr* addr, unsigned flags) {
  uv_loop_t *loop = handle->loop;
  lua_State *L = (lua_State *)loop->data;
  uv_buf_t *bufptr;
  struct sockaddr_in *ip4addr;

printf("udp_recv_cb nread=%d, buf.base=%s, buf.len=%d\n", nread, buf.base, buf.len);
  lua_pushnumber(L, nread);

printf("udp_recv_cb #2\n");
  bufptr = (uv_buf_t *)lua_newuserdata(L, sizeof(uv_buf_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  *bufptr = buf;
printf("udp_recv_cb #3\n");

#if 0
  /* registry[buf] = ref to char[] buf */
  ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_pushlightuserdata(L, bufptr);
  lua_pushnumber(L, ref);
  lua_rawset(L, LUA_REGISTRYINDEX);
#endif

  if (nread == 0) {
    lua_pushnil(L);
  } else {
    ip4addr = lua_newuserdata(L, sizeof(struct sockaddr_in));
  printf("udp_recv_cb #3.1 top=%d\n", lua_gettop(L));
    luaL_getmetatable(L, LUV_IP4ADDR_MTBL_NAME);
  printf("udp_recv_cb #3.2 top=%d\n", lua_gettop(L));
    lua_setmetatable(L, -2);
  printf("udp_recv_cb #3.3 top=%d, addr=%x\n", lua_gettop(L), (unsigned long)addr);
    *ip4addr = *(struct sockaddr_in *)addr;
  printf("udp_recv_cb #4\n");
  }

  lua_resume(L, 3);
printf("udp_recv_cb #5\n");
}

int luv_udp_recv(lua_State *L) {
  int r;
  uv_loop_t *loop;
  uv_udp_t *handle;

printf("udp_recv #1\n");
  loop = luv_loop(L);
printf("udp_recv #2\n");
  handle = (uv_udp_t *)lua_touserdata(L, 1);
printf("udp_recv #3\n");
  r = uv_udp_recv_start(handle, alloc_cb, udp_recv_cb);
printf("udp_recv #4\n");
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
printf("udp_recv #5\n");
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

/*
 * Receive data. If the socket has not previously been bound with `uv_udp_bind`
 * or `uv_udp_bind6`, it is bound to 0.0.0.0 (the "all interfaces" address)
 * and a random port number.
 *
 * Arguments:
 *  handle    UDP handle. Should have been initialized with `uv_udp_init`.
 *  alloc_cb  Callback to invoke when temporary storage is needed.
 *  recv_cb   Callback to invoke with received data.
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_recv_start(uv_udp_t* handle, uv_alloc_cb alloc_cb,
    uv_udp_recv_cb recv_cb);

/*
 * Stop listening for incoming datagrams.
 *
 * Arguments:
 *  handle    UDP handle. Should have been initialized with `uv_udp_init`.
 *
 * Returns:
 *  0 on success, -1 on error.
 */
UV_EXTERN int uv_udp_recv_stop(uv_udp_t* handle);
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
  lua_createtable(L, 0, ARRAY_SIZE(udp_functions) - 1);
  luaL_register(L, NULL, udp_functions);

  lua_setfield(L, -2, "udp");
  return 1;
}
