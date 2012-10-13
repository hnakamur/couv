#ifndef _AUXLIB_H
#define _AUXLIB_H

#include <lua.h>

void *luv_alloc(lua_State *L, size_t size);
void luv_free(lua_State *L, void *ptr);

int luvL_is_in_mainthread(lua_State *L);
int luvL_hasmetatablename(lua_State *L, int index, const char *tname);
const char *luvL_uv_errname(int uv_errcode);

void luv_registry_set_for_ptr(lua_State *L, void *ptr, int index);
void luv_registry_get_for_ptr(lua_State *L, void *ptr);
void luv_registry_delete_for_ptr(lua_State *L, void *ptr);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#define luvL_SET_FIELD(L, name, type, val) \
  lua_push##type(L, val);                  \
  lua_setfield(L, -2, #name)

#endif /* _AUXLIB_H */
