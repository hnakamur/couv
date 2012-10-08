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

static char *get_start_ptr(lua_State *L, int index, luv_buffer_t **buf_ptr) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int start = luaL_optint(L, index, 1);
  luaL_argcheck(L, 1 <= start && start <= buffer->length, index,
      "index out of range");
  if (buf_ptr)
    *buf_ptr = buffer;
  return buffer->buf + start - 1;
}

static char *get_start_ptr_and_length(lua_State *L, int index, int *length,
    luv_buffer_t **buf_ptr) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int start = luaL_optint(L, index, 1);
  int end = luaL_optint(L, index + 1, buffer->length);
  luaL_argcheck(L, 1 <= start && start <= buffer->length, index,
      "index out of range");
  luaL_argcheck(L, start <= end && end <= buffer->length , index + 1,
      "index out of range");
  *length = end - start + 1;
  if (buf_ptr)
    *buf_ptr = buffer;
  return buffer->buf + start - 1;
}

static int buffer_gc(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  luaL_unref(L, LUA_REGISTRYINDEX, buffer->buf_ref);
  return 0;
}

static int buffer_readUInt8(lua_State *L) {
  char *p = get_start_ptr(L, 2, NULL);
  lua_pushnumber(L, *(unsigned char *)p);
  return 1;
}

static int buffer_index(lua_State *L) {
  const char *key = luaL_checkstring(L, 2);
  lua_getmetatable(L, 1);
  lua_getfield(L, -1, key);
  if (!lua_isnil(L, -1))
    return 1;
  lua_pop(L, 1);
  return buffer_readUInt8(L);
}

static int buffer_writeUInt8(lua_State *L) {
  char *p = get_start_ptr(L, 2, NULL);
  lua_Integer byte = luaL_checkinteger(L, 3);
  *(unsigned char *)p = (unsigned char)byte;
  return 0;
}

static int buffer_length(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  lua_pushnumber(L, buffer->length);
  return 1;
}

static int buffer_slice(lua_State *L) {
  luv_buffer_t *buffer;
  int length;
  char *p = get_start_ptr_and_length(L, 2, &length, &buffer);
  luv_buffer_t *slice;

  slice = (luv_buffer_t *)lua_newuserdata(L, sizeof(luv_buffer_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);

  slice->length = length;
  slice->buf = p;

  lua_rawgeti(L, LUA_REGISTRYINDEX, buffer->buf_ref);
  slice->buf_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}

static int buffer_to_string(lua_State *L) {
  int length;
  char *p = get_start_ptr_and_length(L, 2, &length, NULL);

  lua_pushlstring(L, p, length);
  return 1;
}

static const struct luaL_Reg buffer_methods[] = {
  { "__gc", buffer_gc },
  { "__index", buffer_index },
  { "__len", buffer_length },
  { "__newindex", buffer_writeUInt8 },
  { "length", buffer_length },
  { "readUInt8", buffer_readUInt8 },
  { "slice", buffer_slice },
  { "toString", buffer_to_string },
  { "writeUInt8", buffer_writeUInt8 },
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
  luaL_newmetatable(L, LUV_BUFFER_MTBL_NAME);
  luaL_register(L, NULL, buffer_methods);
  lua_pop(L, 1);

  lua_createtable(L, 0, ARRAY_SIZE(buffer_functions) - 1);
  luaL_register(L, NULL, buffer_functions);
  lua_setfield(L, -2, "Buffer");
  return 1;
}
