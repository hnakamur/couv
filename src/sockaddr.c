#include "couv-private.h"

#define COUV_AF_UNSPEC 0
#define COUV_AF_IPV4 4
#define COUV_AF_IPV6 6

/*
 * helper functions
 */

static uv_err_code couv_inet_pton(int *af, const char *src, void *dst) {
  uv_err_t err;

  switch (*af) {
  case AF_UNSPEC:
    err = uv_inet_pton(AF_INET, src, dst);
    if (err.code == UV_OK)
      *af = AF_INET;
    else {
      err = uv_inet_pton(AF_INET6, src, dst);
      if (err.code == UV_OK)
        *af = AF_INET6;
    }
    break;
  default:
    err = uv_inet_pton(*af, src, dst);
    break;
  }
  return err.code;
}

static const char *couvL_checkip(lua_State *L, int index, int *af,
    void *addr) {
  uv_err_code err_code;
  const char *str;

  str = luaL_checkstring(L, index);
  err_code = couv_inet_pton(af, str, addr);
  luaL_argcheck(L, err_code == UV_OK, index, "must be valid IP address");
  return str;
}


static int couvL_checkport(lua_State *L, int index) {
  int port;

  port = luaL_checkint(L, index);
  luaL_argcheck(L, 0 <= port && port <= 65535, index,
      "must be integer between 0 and 65535");
  return port;
}

int couvL_tosockfamily(lua_State *L, int index) {
  int family;

  family = luaL_optint(L, index, AF_UNSPEC);
  switch (family) {
  case AF_UNSPEC:
  case AF_INET:
  case AF_INET6:
    return family;
  default:
    return -1;
  }
}

static int couvL_checksockfamily(lua_State *L, int index) {
  int family;

  family = couvL_tosockfamily(L, index);
  luaL_argcheck(L, family != -1, index,
      "must be AF_UNSPEC, AF_INET or AF_INET6");
  return family;
}


static int couv_sockaddrv4_push_raw(lua_State *L, struct sockaddr_in *addr) {
  struct sockaddr_in *ud_addr;

  ud_addr = lua_newuserdata(L, sizeof(struct sockaddr_in));
  *ud_addr = *addr;
  luaL_getmetatable(L, COUV_SOCK_ADDR_V4_MTBL_NAME);
  lua_setmetatable(L, -2);
  return 1; 
}

static int couv_sockaddrv6_push_raw(lua_State *L, struct sockaddr_in6 *addr) {
  struct sockaddr_in6 *ud_addr;

  ud_addr = lua_newuserdata(L, sizeof(struct sockaddr_in6));
  *ud_addr = *addr;
  luaL_getmetatable(L, COUV_SOCK_ADDR_V6_MTBL_NAME);
  lua_setmetatable(L, -2);
  return 1; 
}

int couv_sockaddr_push_raw(lua_State *L, struct sockaddr *addr) {
  if (addr->sa_family == AF_INET)
    return couv_sockaddrv4_push_raw(L, (struct sockaddr_in *)addr);
  else if (addr->sa_family == AF_INET6)
    return couv_sockaddrv6_push_raw(L, (struct sockaddr_in6 *)addr);
  else
    return luaL_error(L, "ENOTSUP");
}

static int couv_sockaddrv4_new_addr_port(lua_State *L, struct in_addr *addr,
    int port) {
  struct sockaddr_in *addr4;

  addr4 = lua_newuserdata(L, sizeof(struct sockaddr_in));
  luaL_getmetatable(L, COUV_SOCK_ADDR_V4_MTBL_NAME);
  lua_setmetatable(L, -2);

  memset(addr4, 0, sizeof(struct sockaddr_in));
  addr4->sin_family = AF_INET;
  addr4->sin_port = htons(port);
  memcpy(&addr4->sin_addr, addr, sizeof(struct in_addr));

  return 1;
}

static int couv_sockaddrv6_new_addr_port(lua_State *L, struct in6_addr *addr,
    int port) {
  struct sockaddr_in6 *addr6;

  addr6 = lua_newuserdata(L, sizeof(struct sockaddr_in6));
  luaL_getmetatable(L, COUV_SOCK_ADDR_V6_MTBL_NAME);
  lua_setmetatable(L, -2);

  memset(addr6, 0, sizeof(struct sockaddr_in6));
  addr6->sin6_family = AF_INET6;
  addr6->sin6_port = htons(port);
  memcpy(&addr6->sin6_addr, addr, sizeof(struct in6_addr));

  return 1;
}


/*
 * SockAddrV4 methods
 */

int couv_sockaddrv4_host(lua_State *L) {
  struct sockaddr_in *addr;
  char buf[sizeof "255.255.255.255"];
  int r;

  addr = couvL_checkudataclass(L, 1, COUV_SOCK_ADDR_V4_MTBL_NAME);
  r = uv_ip4_name(addr, buf, sizeof(buf));
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  lua_pushstring(L, buf);
  return 1;
}

int couv_sockaddrv4_port(lua_State *L) {
  struct sockaddr_in *addr;

  addr = couvL_checkudataclass(L, 1, COUV_SOCK_ADDR_V4_MTBL_NAME);
  lua_pushnumber(L, ntohs(addr->sin_port));
  return 1;
}

static const struct luaL_Reg sockaddrv4_methods[] = {
  { "host", couv_sockaddrv4_host },
  { "port", couv_sockaddrv4_port },
  { NULL, NULL }
};


/*
 * SockAddrV4 functions
 */

int couv_sockaddrv4_new(lua_State *L) {
  int port;
  int af;
  unsigned char tmp[sizeof(struct in_addr)];

  af = AF_INET;
  couvL_checkip(L, 1, &af, tmp);
  port = couvL_checkport(L, 2);
  return couv_sockaddrv4_new_addr_port(L, (struct in_addr *)tmp, port);
}

static const struct luaL_Reg sockaddrv4_functions[] = {
  { "new", couv_sockaddrv4_new },
  { NULL, NULL }
};


/*
 * SockAddrV6 methods
 */

int couv_sockaddrv6_host(lua_State *L) {
  struct sockaddr_in6 *addr;
  char buf[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
  int r;

  addr = couvL_checkudataclass(L, 1, COUV_SOCK_ADDR_V6_MTBL_NAME);
  r = uv_ip6_name(addr, buf, sizeof(buf));
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  lua_pushstring(L, buf);
  return 1;
}

int couv_sockaddrv6_port(lua_State *L) {
  struct sockaddr_in6 *addr;

  addr = couvL_checkudataclass(L, 1, COUV_SOCK_ADDR_V6_MTBL_NAME);
  lua_pushnumber(L, ntohs(addr->sin6_port));
  return 1;
}

static const struct luaL_Reg sockaddrv6_methods[] = {
  { "host", couv_sockaddrv6_host },
  { "port", couv_sockaddrv6_port },
  { NULL, NULL }
};


/*
 * SockAddrV6 functions
 */

int couv_sockaddrv6_new(lua_State *L) {
  int port;
  int af;
  unsigned char tmp[sizeof(struct in6_addr)];

  af = AF_INET6;
  couvL_checkip(L, 1, &af, tmp);
  port = couvL_checkport(L, 2);
  return couv_sockaddrv6_new_addr_port(L, (struct in6_addr *)tmp, port);
}

static const struct luaL_Reg sockaddrv6_functions[] = {
  { "new", couv_sockaddrv6_new },
  { NULL, NULL }
};

static int couv_sockaddr_is_v4(lua_State *L) {
  struct sockaddr *addr;

  addr = couvL_checkudataclass(L, 1, COUV_SOCK_ADDR_MTBL_NAME);
  lua_pushboolean(L, addr->sa_family == AF_INET);
  return 1; 
}

static int couv_sockaddr_is_v6(lua_State *L) {
  struct sockaddr *addr;

  addr = couvL_checkudataclass(L, 1, COUV_SOCK_ADDR_MTBL_NAME);
  lua_pushboolean(L, addr->sa_family == AF_INET6);
  return 1; 
}

static const struct luaL_Reg sockaddr_methods[] = {
  { "isV4", couv_sockaddr_is_v4 },
  { "isV6", couv_sockaddr_is_v6 },
  { NULL, NULL }
};

int couv_sockaddr_create(lua_State *L) {
  int port;
  int af;
  unsigned char tmp[sizeof(struct in6_addr)];

  port = couvL_checkport(L, 2);
  af = couvL_checksockfamily(L, 3);
  couvL_checkip(L, 1, &af, tmp);

  return af == AF_INET
      ? couv_sockaddrv4_new_addr_port(L, (struct in_addr *)tmp, port)
      : couv_sockaddrv6_new_addr_port(L, (struct in6_addr *)tmp, port);
}

int couv_sockaddr_is_sockaddr(lua_State *L) {
  struct sockaddr *addr;

  addr = couvL_testudataclass(L, 1, COUV_SOCK_ADDR_MTBL_NAME);
  lua_pushboolean(L, addr != NULL);
  return 1; 
}

static const struct luaL_Reg sockaddr_functions[] = {
  { "create", couv_sockaddr_create },
  { "isSockAddr", couv_sockaddr_is_sockaddr },
  { NULL, NULL }
};

int luaopen_couv_sockaddr(lua_State *L) {
  lua_newtable(L);
  couvL_setfuncs(L, sockaddr_functions, 0);
  couvL_SET_FIELD(L, AF_UNSPEC, number, AF_UNSPEC);
  couvL_SET_FIELD(L, AF_INET, number, AF_INET);
  couvL_SET_FIELD(L, AF_INET6, number, AF_INET6);
  couvL_SET_FIELD(L, SOCK_DGRAM, number, SOCK_DGRAM);
  couvL_SET_FIELD(L, SOCK_STREAM, number, SOCK_STREAM);
  couvL_SET_FIELD(L, IPPROTO_TCP, number, IPPROTO_TCP);
  couvL_SET_FIELD(L, IPPROTO_UDP, number, IPPROTO_UDP);
  lua_setfield(L, -2, "SockAddr");

  couv_newmetatable(L, COUV_SOCK_ADDR_MTBL_NAME, NULL);
  couvL_setfuncs(L, sockaddr_methods, 0);
  lua_setfield(L, -2, "_SockAddr");


  lua_newtable(L);
  couvL_setfuncs(L, sockaddrv4_functions, 0);
  lua_setfield(L, -2, "SockAddrV4");

  couv_newmetatable(L, COUV_SOCK_ADDR_V4_MTBL_NAME, COUV_SOCK_ADDR_MTBL_NAME);
  couvL_setfuncs(L, sockaddrv4_methods, 0);
  lua_setfield(L, -2, "_SockAddrV4");


  lua_newtable(L);
  couvL_setfuncs(L, sockaddrv6_functions, 0);
  lua_setfield(L, -2, "SockAddrV6");

  couv_newmetatable(L, COUV_SOCK_ADDR_V6_MTBL_NAME, COUV_SOCK_ADDR_MTBL_NAME);
  couvL_setfuncs(L, sockaddrv6_methods, 0);
  lua_setfield(L, -2, "_SockAddrV6");

  return 0;
}
