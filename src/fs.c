#include "yaluv_priv.h"

/*
typedef struct luv_fs_s {
  lua_State *L;
  int req_ref;
  uv_fs_t req;
} luv_fs_t;
*/

typedef struct flag_mapping_s {
  char *name;
  int flags;
} flag_mapping_t;

static flag_mapping_t flag_mappings[] = {
   { "r",   O_RDONLY                      }
  ,{ "w",   O_WRONLY | O_CREAT | O_TRUNC  }
  ,{ "a",   O_WRONLY | O_CREAT | O_APPEND }
  ,{ "r+",  O_RDWR                        }
  ,{ "w+",  O_RDWR   | O_CREAT | O_TRUNC  }
  ,{ "a+",  O_RDWR   | O_CREAT | O_APPEND }
#ifndef _WIN32
  ,{ "rs",  O_RDONLY | O_SYNC             }
  ,{ "rs+", O_RDWR   | O_SYNC             }
#endif
  ,{ NULL,  0                             }
};

static int fs_toflags(const char *flags_str) {
  flag_mapping_t *mapping;
  for (mapping = flag_mappings; mapping->name; mapping++) {
    if (strcmp(flags_str, mapping->name) == 0) {
      return mapping->flags;
    }
  }
  return -1;
}

static int fs_checkflags(lua_State *L, int index) {
  const char *flags_str = luaL_checkstring(L, index);
  int flags = fs_toflags(flags_str);
  if (flags == -1) {
    return luaL_argerror(L, index, "Invalid flags");
  }
  return flags;
}

static int fs_checkmode(lua_State *L, int index, int default_value) {
  int type = lua_type(L, index);
  int mode;
  if (type == LUA_TNUMBER) {
    mode = lua_tonumber(L, index);
  } else if (type == LUA_TSTRING) {
    size_t len;
    const char *str = lua_tolstring(L, index, &len);
    mode = strtoul(str, NULL, 8);
  } else if (type == LUA_TNIL || type == LUA_TNONE) {
    mode = default_value;
  } else {
    return luaL_argerror(L, index, "must be octal number string or number");
  }
  return mode;
}

static void fs_common_callback(uv_fs_t* req) {
  lua_State *L = req->data;
  int nresults;

#if 0
  /* release coroutine */
  lua_pushthread(L);
  lua_pushnil(L);
  lua_rawset(L, LUA_REGISTRYINDEX);
#endif

  if (req->errorno) {
    lua_pushstring(L, luv_uv_errname(req->errorno));
    nresults = 1;
  } else {
    lua_pushnil(L);
    lua_pushnumber(L, req->result);
    nresults = 2;
  }
  uv_fs_req_cleanup(req);
  free(req);
  lua_resume(L, nresults);
}

static int fs_open(lua_State *L) {
  uv_loop_t **loop = luv_checkloop(L, 1);
  const char *path = luaL_checkstring(L, 2);
  int flags = fs_checkflags(L, 3);
  int mode = fs_checkmode(L, 4, 0666);
  int is_mainthread = lua_pushthread(L);
  if (is_mainthread) {
    uv_fs_t req;
    int r;

    lua_pop(L, 1);
    r = uv_fs_open(*loop, &req, path, flags, mode, NULL);
    uv_fs_req_cleanup(&req);
    if (r < 0) {
      int errcode = uv_last_error(*loop).code;
      lua_pushstring(L, luv_uv_errname(errcode));
      return 1;
    }

    lua_pushnil(L);
    lua_pushnumber(L, r);
    return 2;
  } else {
    uv_fs_t *req;

#if 1
    lua_pop(L, 1);
#else
    /* map coroutine to a pointer to prevent gc */
    lua_pushlightuserdata(L, L);
    lua_rawset(L, LUA_REGISTRYINDEX);
#endif

    req = (uv_fs_t *)malloc(sizeof(uv_fs_t));
    req->data = L;

    uv_fs_open(*loop, req, path, flags, mode, fs_common_callback);
    return lua_yield(L, 0);
  }
}

static const struct luaL_Reg fs_functions[] = {
  { "open", fs_open },
  { NULL, NULL }
};

int luaopen_yaluv_fs(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(fs_functions) - 1);
  luaL_register(L, NULL, fs_functions);

  lua_setfield(L, -2, "fs");
  return 1;
}
