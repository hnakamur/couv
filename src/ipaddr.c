#include "yaluv-private.h"


int ip4addr_dbg_print(const char *header, struct sockaddr_in *addr) {
  char buf[sizeof "255.255.255.255"];
  int r;
  if (addr) {
    unsigned char *p;
    r = uv_ip4_name(addr, buf, sizeof(buf));
    if (r < 0) {
      printf("%s error in uv_ip4_name r=%d\n", header, r);
    }
    p = (unsigned char *)&addr->sin_port;
    printf("%s addr host=%s, port=%d\n", header, buf,  p[0] << 8 | p[1]);
  } else {
    printf("%s addr=NULL\n", header);
  }
  return r;
}

int luv_ip4addr_host(lua_State *L) {
  struct sockaddr_in *addr = luv_checkip4addr(L, 1);
  char buf[sizeof "255.255.255.255"];
  int r = uv_ip4_name(addr, buf, sizeof(buf));
  if (r < 0) {
    uv_loop_t *loop = luv_loop(L);
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  lua_pushstring(L, buf);
  return 1;
}

int luv_ip4addr_port(lua_State *L) {
  struct sockaddr_in *addr = luv_checkip4addr(L, 1);
  unsigned char *p = (unsigned char *)&addr->sin_port;
  lua_pushnumber(L, p[0] << 8 | p[1]);
  return 1;
}

static const struct luaL_Reg ip4addr_methods[] = {
  { "host", luv_ip4addr_host },
  { "port", luv_ip4addr_port },
  { NULL, NULL }
};


int luv_ip6addr_host(lua_State *L) {
  struct sockaddr_in6 *addr = luv_checkip6addr(L, 1);
  char buf[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"];
  int r = uv_ip6_name(addr, buf, sizeof(buf));
  if (r < 0) {
    uv_loop_t *loop = luv_loop(L);
    return luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
  }
  lua_pushstring(L, buf);
  return 1;
}

int luv_ip6addr_port(lua_State *L) {
  struct sockaddr_in6 *addr = luv_checkip6addr(L, 1);
  unsigned char *p = (unsigned char *)&addr->sin6_port;
  lua_pushnumber(L, p[0] << 8 | p[1]);
  return 1;
}

static const struct luaL_Reg ip6addr_methods[] = {
  { "host", luv_ip6addr_host },
  { "port", luv_ip6addr_port },
  { NULL, NULL }
};


int luv_ip4addr(lua_State *L) {
  const char *ip = luaL_checkstring(L, 1);
  int port = luaL_checkint(L, 2);
  struct sockaddr_in *addr = lua_newuserdata(L, sizeof(struct sockaddr_in));
  luaL_getmetatable(L, LUV_IP4ADDR_MTBL_NAME);
  lua_setmetatable(L, -2);
  *addr = uv_ip4_addr(ip, port);
  return 1; 
}

int luv_ip6addr(lua_State *L) {
  const char *ip = luaL_checkstring(L, 1);
  int port = luaL_checkint(L, 2);
  struct sockaddr_in6 *addr = lua_newuserdata(L, sizeof(struct sockaddr_in6));
  luaL_getmetatable(L, LUV_IP6ADDR_MTBL_NAME);
  lua_setmetatable(L, -2);
  *addr = uv_ip6_addr(ip, port);
  return 1; 
}

static const struct luaL_Reg ipaddr_functions[] = {
  { "ip4addr", luv_ip4addr },
  { "ip6addr", luv_ip6addr },
  { NULL, NULL }
};

int luaopen_yaluv_ipaddr(lua_State *L) {
  luaL_register(L, NULL, ipaddr_functions);

  luaL_newmetatable(L, LUV_IP4ADDR_MTBL_NAME);
  luaL_register(L, NULL, ip4addr_methods);
  lua_setfield(L, -1, "__index");

  luaL_newmetatable(L, LUV_IP6ADDR_MTBL_NAME);
  luaL_register(L, NULL, ip6addr_methods);
  lua_setfield(L, -1, "__index");
  return 1;
}
