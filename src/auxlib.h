#ifndef _AUXLIB_H
#define _AUXLIB_H

#include <lua.h>

int luvL_is_in_mainthread(lua_State *L);
const char *luvL_uv_errname(int uv_errcode);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#endif /* _AUXLIB_H */
