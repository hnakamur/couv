#include <lauxlib.h>
#include <stdlib.h>
#include <uv.h>

#include "auxlib.h"
#include "buffer.h"
#include "ipaddr.h"
#include "loop.h"
#include "udp.h"

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
printf("create handle=%x\n", handle);
  int r = uv_udp_init(loop, handle);
printf("create r=%d\n", r);
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

static int print_addr(const char *header, struct sockaddr_in *addr) {
  char buf[sizeof "255.255.255.255"];
  if (addr) {
    int r = uv_ip4_name(addr, buf, sizeof(buf));
    unsigned char *p = (unsigned char *)&addr->sin_port;
    printf("%s addr host=%s, port=%d\n", header, buf,  p[0] << 8 | p[1]);
  } else {
    printf("%s addr=NULL\n", header);
  }
}

int luv_udp_bind(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  uv_udp_t *handle = (uv_udp_t *)lua_touserdata(L, 1);
printf("bind handle=%x\n", handle);
  struct sockaddr_in *addr = luv_checkip4addr(L, 2);
print_addr("bind", addr);
  int r = uv_udp_bind(handle, *addr, 0);
printf("bind r=%d\n", r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  return 0;
}

static uv_buf_t sendbuf;
static uv_buf_t *bufs;

static void udp_send_cb(uv_udp_send_t* req, int status) {
  luv_udp_send_t *holder;
  uv_loop_t *loop;
  lua_State *L;
  int r;
printf("udp_send_cb status=%d\n", status);
  holder = container_of(req, luv_udp_send_t, req);
  L = holder->L;
  loop = req->handle->loop;
printf("udp_send_cb loop=%x, L=%x\n", (unsigned long)loop, (unsigned long)L);
#if 0
  free(req->data);
#endif
  if (status < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
#if 1
  lua_resume(L, 0);
#else
  r = lua_resume(L, 0);
printf("udp_send_cb resume result=%d, LUA_YIELD=%d, LUA_ERRRUN=%d, LUA_ERRMEME=%d, LUA_ERRERR=%d\n", r, LUA_YIELD, LUA_ERRRUN, LUA_ERRMEM, LUA_ERRERR);
  if (r) {
    const char *errmsg = lua_tostring(L, -1);
    printf("errmsg=%s\n", errmsg);
  }
#endif
}

int luv_udp_send(lua_State *L) {
  int r;
  uv_loop_t *loop;
  uv_udp_t *handle;
  struct sockaddr_in *addr;
  size_t bufcnt;
  int i;
  luv_udp_send_t *holder;

printf("udp_send #1 top=%d\n", lua_gettop(L));
  loop = luv_loop(L);
printf("udp_send loop=%x, L=%x\n", (unsigned long)loop, (unsigned long)L);
  handle = (uv_udp_t *)lua_touserdata(L, 1);
printf("send handle=%x\n", handle);
  addr = luv_checkip4addr(L, 2);
print_addr("send", addr);

  bufs = luv_checkbuforstrtable(L, 3, &bufcnt);
printf("udp_send #1.1 bufcnt=%d\n", bufcnt);
for (i = 0; i < bufcnt; ++i) {
printf("i=%d, buf.len=%d, buf.base=%s\n", i, bufs[i].len, bufs[i].base);
}
  holder = luv_alloc_udp_send(L, bufs);
printf("udp_send #2\n");
  r = uv_udp_send(&holder->req, handle, bufs, (int)bufcnt, *addr, udp_send_cb);
printf("udp_send #3 r=%d\n", r);
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
printf("udp_send #4 top=%d\n", lua_gettop(L));
  return lua_yield(L, 0);
}

static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  uv_loop_t *loop;
  lua_State *L;
  uv_buf_t buf;
printf("alloc_cb #1 suggested_size=%u\n", suggested_size);
  loop = handle->loop;
  L = (lua_State *)handle->data;
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
  uv_loop_t *loop;
  lua_State *L;
  uv_buf_t *bufptr;
  struct sockaddr_in *ip4addr;

printf("ucb_recv_cb handle=%x\n", handle);
  loop = handle->loop;
  L = (lua_State *)handle->data;
printf("udp_recv_cb loop=%x, L=%x\n", (unsigned long)loop, (unsigned long)L);
printf("udp_recv_cb nread=%d, buf.base=%s, buf.len=%d\n", nread, buf.base, buf.len);
print_addr("udp_recv_cb", addr);
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

#if 1
  lua_resume(L, 3);
#else
  return 3;
#endif
}

int luv_udp_recv(lua_State *L) {
  int r;
  uv_loop_t *loop;
  uv_udp_t *handle;

printf("udp_recv #1\n");
  loop = luv_loop(L);
printf("udp_recv #2\n");
  handle = (uv_udp_t *)lua_touserdata(L, 1);
  handle->data = L;
printf("udp_recv #3 handle=%x\n", handle);
  r = uv_udp_recv_start(handle, alloc_cb, udp_recv_cb);
printf("udp_recv #4\n");
  if (r < 0) {
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
printf("udp_recv #5\n");
#if 1
  return lua_yield(L, 0);
#else
  return 0;
#endif
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
