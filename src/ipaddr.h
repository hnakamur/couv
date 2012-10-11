#ifndef _IPADDR_H
#define _IPADDR_H

#include <sys/socket.h>
#include <lua.h>

#define LUV_IP4ADDR_MTBL_NAME "luv.Ip4addr"
#define luv_checkip4addr(L, index) \
  (struct sockaddr_in *)luaL_checkudata(L, index, LUV_IP4ADDR_MTBL_NAME)

#define LUV_IP6ADDR_MTBL_NAME "luv.Ip6addr"
#define luv_checkip6addr(L, index) \
  (struct sockaddr_in6 *)luaL_checkudata(L, index, LUV_IP6ADDR_MTBL_NAME)

int luaopen_yaluv_ipaddr(lua_State *L);

#endif /* _IPADDR_H */
