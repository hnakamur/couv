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


int couv_newmetatable(lua_State *L, const char *tname,
    const char *super_tname) {
  luaL_newmetatable(L, tname);

  /* metatable.__index = metatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /* setmatable(metatable, super_metatable) */
  if (super_tname) {
    lua_getfield(L, LUA_REGISTRYINDEX, super_tname);
    lua_setmetatable(L, -2);
  }

  return 1;
}

int couv_absindex(lua_State *L, int idx) {
  return idx < 0 ? lua_gettop(L) + idx + 1 : idx;
}

#if LUA_VERSION_NUM == 501

void couvL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
  for (; l->name; ++l) {
    lua_pushcfunction(L, l->func);
    lua_setfield(L, -2, l->name);
  }
}

void *couvL_testudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      if (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      lua_pop(L, 2);  /* remove both metatables */
      return p;
    }
  }
  return NULL;  /* value is not a userdata with a metatable */
}

void couv_rawsetp(lua_State *L, int index, const void *p) {
  lua_pushlightuserdata(L, (void *)p);
  lua_insert(L, -2);
  lua_rawset(L, index);
}

void couv_rawgetp(lua_State *L, int index, const void *p) {
  lua_pushlightuserdata(L, (void *)p);
  lua_rawget(L, index);
}

#endif
