#include <lauxlib.h>
#include "auxlib.h"
#include "buffer.h"

typedef struct luv_buffer_s {
  int length;
  int buf_ref;
  char *buf;
} luv_buffer_t;

#define LUV_BUFFER_MTBL_NAME "luv.Buffer"
#define luv_checkbuffer(L, index) \
    (luv_buffer_t *)luaL_checkudata(L, index, LUV_BUFFER_MTBL_NAME)

static void checkarg_index(lua_State *L, luv_buffer_t *buffer,
    int arg_index, int index) {
  luaL_argcheck(L, 1 <= index && index <= buffer->length, arg_index,
      "index out of range");
}

static void checkarg_first_last(lua_State *L, luv_buffer_t *buffer,
    int first_index, int last_index, int first, int last) {
  checkarg_index(L, buffer, first_index, first);
  checkarg_index(L, buffer, last_index, last);
  luaL_argcheck(L, first <= last, last_index,
      "last must be greater than or equal to first");
}

static int buffer__gc(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  luaL_unref(L, LUA_REGISTRYINDEX, buffer->buf_ref);
  return 0;
}

static int buffer_readUInt8(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int index = luaL_checkint(L, 2);
  checkarg_index(L, buffer, 2, index);

  lua_pushnumber(L, ((unsigned char *)buffer->buf)[index - 1]);
  return 1;
}

static int buffer__newindex(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int index = luaL_checkint(L, 2);
  int byte = luaL_checkint(L, 3);
  checkarg_index(L, buffer, 2, index);

  ((unsigned char *)buffer->buf)[index - 1] = (unsigned char)byte;
  return 0;
}

static int buffer__len(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  lua_pushnumber(L, buffer->length);
  return 1;
}

static int buffer_slice(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int first = luaL_optint(L, 2, 1);
  int last = luaL_optint(L, 3, buffer->length);
  luv_buffer_t *slice;
  checkarg_first_last(L, buffer, 2, 3, first, last);

  slice = (luv_buffer_t *)lua_newuserdata(L, sizeof(luv_buffer_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  slice->length = last - first + 1;
  slice->buf = buffer->buf + first - 1;
  lua_rawgeti(L, LUA_REGISTRYINDEX, buffer->buf_ref);
  slice->buf_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}

static int buffer_to_string(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int first = luaL_optint(L, 2, 1);
  int last = luaL_optint(L, 3, buffer->length);
  checkarg_first_last(L, buffer, 2, 3, first, last);

  lua_pushlstring(L, buffer->buf + first - 1, last - first + 1);
  return 1;
}

static const struct luaL_Reg buffer_methods[] = {
  { "__gc", buffer__gc },
  { "__index", buffer_readUInt8 },
  { "__len", buffer__len },
  { "__newindex", buffer__newindex },
  { "readUInt8", buffer_readUInt8 },
  { "slice", buffer_slice },
  { "toString", buffer_to_string },
  { NULL, NULL }
};

static int buffer_new(lua_State *L) {
  int length = luaL_checkint(L, 1);
  luv_buffer_t *buffer = (luv_buffer_t *)lua_newuserdata(L,
      sizeof(luv_buffer_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  buffer->length = length;
  buffer->buf = (char *)lua_newuserdata(L, length);
  buffer->buf_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}

static const struct luaL_Reg buffer_functions[] = {
  { "new", buffer_new },
  { NULL, NULL }
};

int luaopen_yaluv_buffer(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(buffer_functions) - 1);
  luaL_register(L, NULL, buffer_functions);

  luaL_newmetatable(L, LUV_BUFFER_MTBL_NAME);
  luaL_register(L, NULL, buffer_methods);
  lua_setfield(L, -1, "__index");

  lua_setfield(L, -2, "Buffer");
  return 1;
}
