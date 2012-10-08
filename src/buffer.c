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

#define luv_argcheckindex(L, arg_index, index, min, max) \
  luaL_argcheck(L, min <= index && index <= max, arg_index, \
      "index out of range");

#define FLOAT_SIZE ((int)sizeof(float))
#define DOUBLE_SIZE ((int)sizeof(double))

static void memcpy_le(unsigned char *src, unsigned char *dst, int length) {
  int i;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  for (i = 0; i < length; ++i)
    *dst++ = *src++;
#elif __BYTE_ORDER == __BIG_ENDIAN
  src += length - 1;
  for (i = 0; i < length; ++i)
    *dst++ = *src--;
#else
#error
#endif
}

static void memcpy_be(unsigned char *src, unsigned char *dst, int length) {
  int i;
#if __BYTE_ORDER == __LITTLE_ENDIAN
  src += length - 1;
  for (i = 0; i < length; ++i)
    *dst++ = *src--;
#elif __BYTE_ORDER == __BIG_ENDIAN
  for (i = 0; i < length; ++i)
    *dst++ = *src++;
#else
#error
#endif
}

static int buffer_gc(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  luaL_unref(L, LUA_REGISTRYINDEX, buffer->buf_ref);
  return 0;
}

static int buffer_read_uint8(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length);

  lua_pushnumber(L, *(unsigned char *)&buffer->buf[position - 1]);
  return 1;
}

static int buffer_read_uint16le(lua_State *L) {
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  p = (unsigned char *)&buffer->buf[position - 1];
  lua_pushnumber(L, p[1] << 8 | p[0]);
  return 1;
}

static int buffer_read_uint16be(lua_State *L) {
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  p = (unsigned char *)&buffer->buf[position - 1];
  lua_pushnumber(L, p[0] << 8 | p[1]);
  return 1;
}

static int buffer_read_uint32le(lua_State *L) {
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  p = (unsigned char *)&buffer->buf[position - 1];
  lua_pushnumber(L, (unsigned)p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0]);
  return 1;
}

static int buffer_read_uint32be(lua_State *L) {
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  p = (unsigned char *)&buffer->buf[position - 1];
  lua_pushnumber(L, (unsigned)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]);
  return 1;
}

static int buffer_read_float_le(lua_State *L) {
  float val;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (FLOAT_SIZE - 1));

  memcpy_le((unsigned char *)&buffer->buf[position - 1], (unsigned char *)&val,
      FLOAT_SIZE);
  lua_pushnumber(L, val);
  return 1;
}

static int buffer_read_float_be(lua_State *L) {
  float val;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (FLOAT_SIZE - 1));

  memcpy_be((unsigned char *)&buffer->buf[position - 1], (unsigned char *)&val,
      FLOAT_SIZE);
  lua_pushnumber(L, val);
  return 1;
}

static int buffer_read_double_le(lua_State *L) {
  double val;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (DOUBLE_SIZE - 1));

  memcpy_le((unsigned char *)&buffer->buf[position - 1], (unsigned char *)&val,
      DOUBLE_SIZE);
  lua_pushnumber(L, val);
  return 1;
}

static int buffer_read_double_be(lua_State *L) {
  double val;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (DOUBLE_SIZE - 1));

  memcpy_be((unsigned char *)&buffer->buf[position - 1], (unsigned char *)&val,
      DOUBLE_SIZE);
  lua_pushnumber(L, val);
  return 1;
}

static int buffer_index(lua_State *L) {
  const char *key = luaL_checkstring(L, 2);
  lua_getmetatable(L, 1);
  lua_getfield(L, -1, key);
  if (!lua_isnil(L, -1))
    return 1;
  lua_pop(L, 1);
  return buffer_read_uint8(L);
}

static int buffer_write_uint8(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  int byte = luaL_checkint(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length);

  *(unsigned char *)&buffer->buf[position - 1] = (unsigned char)byte;
  return 0;
}

static int buffer_length(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  lua_pushnumber(L, buffer->length);
  return 1;
}

static int buffer_slice(lua_State *L) {
  luv_buffer_t *slice;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int first = luaL_optint(L, 2, 1);
  int last = luaL_optint(L, 3, buffer->length);
  luv_argcheckindex(L, 2, first, 1, buffer->length);
  luv_argcheckindex(L, 3, last, first, buffer->length);

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
  luv_argcheckindex(L, 2, first, 1, buffer->length);
  luv_argcheckindex(L, 3, last, first, buffer->length);

  lua_pushlstring(L, buffer->buf + first - 1, last - first + 1);
  return 1;
}

static const struct luaL_Reg buffer_methods[] = {
  { "__gc", buffer_gc },
  { "__index", buffer_index },
  { "__len", buffer_length },
  { "__newindex", buffer_write_uint8 },
  { "length", buffer_length },
  { "readDoubleLE", buffer_read_double_le },
  { "readDoubleBE", buffer_read_double_be },
  { "readFloatLE", buffer_read_float_le },
  { "readFloatBE", buffer_read_float_be },
  { "readUInt8", buffer_read_uint8 },
  { "readUInt16LE", buffer_read_uint16le },
  { "readUInt16BE", buffer_read_uint16be },
  { "readUInt32LE", buffer_read_uint32le },
  { "readUInt32BE", buffer_read_uint32be },
  { "slice", buffer_slice },
  { "toString", buffer_to_string },
  { "writeUInt8", buffer_write_uint8 },
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
