#include "couv-private.h"

void *couv_alloc(lua_State *L, size_t size) {
#if 1
  void *p;

  p = malloc(size);
  if (!p) {
    luaL_error(L, "ENOMEM");
  }
  return p;
#else
  lua_Alloc f;
  void *p;

  f = lua_getallocf(L, NULL);
  p = f(NULL, NULL, 0, size);
  if (!p) {
    luaL_error(L, "ENOMEM");
  }
  return p;
#endif
}

void couv_free(lua_State *L, void *ptr) {
#if 1
  free(ptr);
#else
  lua_Alloc f;

  f = lua_getallocf(L, NULL);
  f(NULL, ptr, 0, 0);
#endif
}

int couvL_is_mainthread(lua_State *L) {
  int is_mainthread = lua_pushthread(L);
  if (is_mainthread)
    lua_pop(L, 1);
  return is_mainthread;
}

int couvL_hasmetatablename(lua_State *L, int index, const char *tname) {
  if (lua_getmetatable(L, index)) {
    luaL_getmetatable(L, tname);
    if (lua_rawequal(L, -1, -2)) {
      lua_pop(L, 2);
      return 1;
    }
    lua_pop(L, 2);
  }
  return 0;
}

#define COUV_UV_ERRNAME_GEN(val, name, s) case val: return #name;

const char *couvL_uv_errname(int uv_errcode) {
  switch (uv_errcode) {
  UV_ERRNO_MAP(COUV_UV_ERRNAME_GEN)
  default: return "UNKNOWN";
  }
}

int couv_registry_set_for_ptr(lua_State *L, void *ptr, int index) {
  lua_pushlightuserdata(L, ptr);
  lua_pushvalue(L, index);
  lua_rawset(L, LUA_REGISTRYINDEX);
  return 0;
}

int couv_registry_get_for_ptr(lua_State *L, void *ptr) {
  lua_pushlightuserdata(L, ptr);
  lua_rawget(L, LUA_REGISTRYINDEX);
  return 1;
}

int couv_registry_delete_for_ptr(lua_State *L, void *ptr) {
  lua_pushlightuserdata(L, ptr);
  lua_pushnil(L);
  lua_rawset(L, LUA_REGISTRYINDEX);
  return 0;
}

#if LUA_VERSION_NUM == 501
void couvL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
  for (; l->name; ++l) {
    lua_pushcfunction(L, l->func);
    lua_setfield(L, -2, l->name);
  }
}
#endif
