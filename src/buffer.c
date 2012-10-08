#include <string.h>
#include <lauxlib.h>
#include "auxlib.h"
#include "buffer.h"

#define FLOAT_SIZE ((int)sizeof(float))
#define DOUBLE_SIZE ((int)sizeof(double))

const char *luv_checkbuforstr(lua_State *L, int index, size_t *length) {
  int type = lua_type(L, index);
  if (type == LUA_TSTRING) {
    return lua_tolstring(L, index, length);
  } else if (luvL_checkmetatablename(L, index, LUV_BUFFER_MTBL_NAME)) {
    luv_buffer_t *buffer = (luv_buffer_t *)lua_touserdata(L, index);
    *length = buffer->length;
    return buffer->buf;
  } else {
    luaL_argerror(L, index, "must be string or Buffer");
  }
}

static void memcpy_le(char *src, char *dst, int length) {
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

static void memcpy_be(char *src, char *dst, int length) {
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

static int buffer_read_int8(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length);

  lua_pushnumber(L, buffer->buf[position - 1]);
  return 1;
}

static int buffer_read_int16le(lua_State *L) {
  int value;
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  p = (unsigned char *)&buffer->buf[position - 1];
  value = p[1] << 8 | p[0];
  if (value >= 0x8000)
    value = -(0xFFFF - value + 1);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_int16be(lua_State *L) {
  int value;
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  p = (unsigned char *)&buffer->buf[position - 1];
  value = p[0] << 8 | p[1];
  if (value >= 0x8000)
    value = -(0xFFFF - value + 1);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_int32le(lua_State *L) {
  unsigned int value;
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  p = (unsigned char *)&buffer->buf[position - 1];
  value = p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0];
  lua_pushnumber(L, value >= 0x80000000
      ? -(int)(0xFFFFFFFF - value + 1) : (int)value);
  return 1;
}

static int buffer_read_int32be(lua_State *L) {
  unsigned int value;
  unsigned char *p;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  p = (unsigned char *)&buffer->buf[position - 1];
  value = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
  lua_pushnumber(L, value >= 0x80000000 ?
      -(int)(0xFFFFFFFF - value + 1) : (int)value);
  return 1;
}

static int buffer_read_float_le(lua_State *L) {
  float value;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (FLOAT_SIZE - 1));

  memcpy_le(&buffer->buf[position - 1], (char *)&value, FLOAT_SIZE);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_float_be(lua_State *L) {
  float value;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (FLOAT_SIZE - 1));

  memcpy_be(&buffer->buf[position - 1], (char *)&value, FLOAT_SIZE);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_double_le(lua_State *L) {
  double value;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (DOUBLE_SIZE - 1));

  memcpy_le(&buffer->buf[position - 1], (char *)&value, DOUBLE_SIZE);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_double_be(lua_State *L) {
  double value;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (DOUBLE_SIZE - 1));

  memcpy_be(&buffer->buf[position - 1], (char *)&value, DOUBLE_SIZE);
  lua_pushnumber(L, value);
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

static int buffer_write_uint16le(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = value & 0xFF;
  *q++ = value >> 8;
  return 0;
}

static int buffer_write_uint16be(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = value >> 8;
  *q++ = value & 0xFF;
  return 0;
}

static int buffer_write_uint32le(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = value & 0xFF;
  *q++ = (value >> 8) & 0xFF;
  *q++ = (value >> 16) & 0xFF;
  *q++ = value >> 24;
  return 0;
}

static int buffer_write_uint32be(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = value >> 24;
  *q++ = (value >> 16) & 0xFF;
  *q++ = (value >> 8) & 0xFF;
  *q++ = value & 0xFF;
  return 0;
}

static int buffer_write_int8(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  int byte = luaL_checkint(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length);

  buffer->buf[position - 1] = (char)byte;
  return 0;
}

static int buffer_write_int16le(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x10000 + value : value;
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = uvalue & 0xFF;
  *q++ = uvalue >> 8;
  return 0;
}

static int buffer_write_int16be(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x10000 + value : value;
  luv_argcheckindex(L, 2, position, 1, buffer->length - 1);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = uvalue >> 8;
  *q++ = uvalue & 0xFF;
  return 0;
}

static int buffer_write_int32le(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x100000000 + value : value;
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = uvalue & 0xFF;
  *q++ = (uvalue >> 8) & 0xFF;
  *q++ = (uvalue >> 16) & 0xFF;
  *q++ = uvalue >> 24;
  return 0;
}

static int buffer_write_int32be(lua_State *L) {
  unsigned char *q;
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x100000000 + value : value;
  luv_argcheckindex(L, 2, position, 1, buffer->length - 3);

  q = (unsigned char *)&buffer->buf[position - 1];
  *q++ = uvalue >> 24;
  *q++ = (uvalue >> 16) & 0xFF;
  *q++ = (uvalue >> 8) & 0xFF;
  *q++ = uvalue & 0xFF;
  return 0;
}

static int buffer_write_float_le(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  float value = (float)luaL_checknumber(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (FLOAT_SIZE - 1));

  memcpy_le((char *)&value, &buffer->buf[position - 1], FLOAT_SIZE);
  return 0;
}

static int buffer_write_float_be(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  float value = (float)luaL_checknumber(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (FLOAT_SIZE - 1));

  memcpy_be((char *)&value, &buffer->buf[position - 1], FLOAT_SIZE);
  return 0;
}

static int buffer_write_double_le(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  double value = (double)luaL_checknumber(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (DOUBLE_SIZE - 1));

  memcpy_le((char *)&value, &buffer->buf[position - 1], DOUBLE_SIZE);
  return 0;
}

static int buffer_write_double_be(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int position = luaL_checkint(L, 2);
  double value = (double)luaL_checknumber(L, 3);
  luv_argcheckindex(L, 2, position, 1, buffer->length - (DOUBLE_SIZE - 1));

  memcpy_be((char *)&value, &buffer->buf[position - 1], DOUBLE_SIZE);
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

static int buffer_fill(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  int byte = luaL_checkint(L, 2);
  int first = luaL_optint(L, 3, 1);
  int last = luaL_optint(L, 4, buffer->length);
  luv_argcheckindex(L, 3, first, 1, buffer->length);
  luv_argcheckindex(L, 4, last, first, buffer->length);

  memset(buffer->buf + first - 1, byte, last - first + 1);
  return 0;
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

static int buffer_copy(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  luv_buffer_t *target = luv_checkbuffer(L, 2);
  int target_first = luaL_optint(L, 3, 1);
  int source_first = luaL_optint(L, 4, 1);
  int source_last = luaL_optint(L, 5, buffer->length);
  int length;
  luv_argcheckindex(L, 3, target_first, 1, target->length);
  luv_argcheckindex(L, 4, source_first, 1, buffer->length);
  luv_argcheckindex(L, 5, source_last, source_first, buffer->length);

  length = source_last - source_first + 1;
  if (length > target->length - target_first + 1)
    length = target->length - target_first + 1;
  if (length > 0)
    memmove(&target->buf[target_first - 1],
        &buffer->buf[source_first - 1], length);
  lua_pushnumber(L, length);
  return 1;
}

static int buffer_write(lua_State *L) {
  luv_buffer_t *buffer = luv_checkbuffer(L, 1);
  size_t str_len;
  const char *str = luaL_checklstring(L, 2, &str_len);
  int position = luaL_optint(L, 3, 1);
  int length = luaL_optint(L, 4, buffer->length - position + 1);
  luv_argcheckindex(L, 3, position, 1, buffer->length);
  luv_argcheckindex(L, 4, length, 0, buffer->length - position + 1);

  if (length > (int)str_len)
    length = (int)str_len;
  if (length > 0)
    memcpy(&buffer->buf[position - 1], str, length);
  lua_pushnumber(L, length);
  return 1;
}

static const struct luaL_Reg buffer_methods[] = {
  { "__gc", buffer_gc },
  { "__index", buffer_index },
  { "__len", buffer_length },
  { "__newindex", buffer_write_uint8 },
  { "copy", buffer_copy },
  { "fill", buffer_fill },
  { "length", buffer_length },
  { "readDoubleBE", buffer_read_double_be },
  { "readDoubleLE", buffer_read_double_le },
  { "readFloatBE", buffer_read_float_be },
  { "readFloatLE", buffer_read_float_le },
  { "readInt8", buffer_read_int8 },
  { "readInt16BE", buffer_read_int16be },
  { "readInt16LE", buffer_read_int16le },
  { "readInt32BE", buffer_read_int32be },
  { "readInt32LE", buffer_read_int32le },
  { "readUInt8", buffer_read_uint8 },
  { "readUInt16BE", buffer_read_uint16be },
  { "readUInt16LE", buffer_read_uint16le },
  { "readUInt32BE", buffer_read_uint32be },
  { "readUInt32LE", buffer_read_uint32le },
  { "slice", buffer_slice },
  { "toString", buffer_to_string },
  { "write", buffer_write },
  { "writeDoubleBE", buffer_write_double_be },
  { "writeDoubleLE", buffer_write_double_le },
  { "writeFloatBE", buffer_write_float_be },
  { "writeFloatLE", buffer_write_float_le },
  { "writeInt8", buffer_write_int8 },
  { "writeInt16BE", buffer_write_int16be },
  { "writeInt16LE", buffer_write_int16le },
  { "writeInt32BE", buffer_write_int32be },
  { "writeInt32LE", buffer_write_int32le },
  { "writeUInt8", buffer_write_uint8 },
  { "writeUInt16BE", buffer_write_uint16be },
  { "writeUInt16LE", buffer_write_uint16le },
  { "writeUInt32BE", buffer_write_uint32be },
  { "writeUInt32LE", buffer_write_uint32le },
  { NULL, NULL }
};

static int buffer_is_buffer(lua_State *L) {
  lua_pushboolean(L, luvL_checkmetatablename(L, 1, LUV_BUFFER_MTBL_NAME));
  return 1;
}

static int buffer_new(lua_State *L) {
  luv_buffer_t *buffer;
  size_t length;
  const char *str = NULL;
  int arg1_type = lua_type(L, 1);
  if (arg1_type == LUA_TNUMBER) {
    length = (size_t)lua_tonumber(L, 1);
  } else if (arg1_type == LUA_TSTRING) {
    str = lua_tolstring(L, 1, &length);
  } else {
    luaL_argerror(L, 1, "must be integer or string");
  }

  buffer = (luv_buffer_t *)lua_newuserdata(L, sizeof(luv_buffer_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  buffer->length = length;
  buffer->buf = (char *)lua_newuserdata(L, length);
  if (str)
    memcpy(buffer->buf, str, length);
  buffer->buf_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 1;
}

static int buffer_concat(lua_State *L) {
  int i;
  int n;
  int total_length;
  luv_buffer_t *buffer;
  luv_buffer_t *target;
  char *dst;
  int len;

  luaL_checktype(L, 1, LUA_TTABLE);
  total_length = luaL_optint(L, 2, 0);
  n = lua_objlen(L, 1);

  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  /* stack: mtbl [total_length] list */

  if (total_length == 0) {
    for (i = 1; i <= n; ++i) {
      lua_rawgeti(L, 1, i);
      buffer = (luv_buffer_t *)lua_touserdata(L, -1);
      if (lua_getmetatable(L, -1) && lua_rawequal(L, -1, -3)) {
        total_length += buffer->length;
        lua_pop(L, 2);
      } else {
        return luaL_argerror(L, 1, "must be an array (table) of buffers");
      }
    }
  }

  lua_pushcfunction(L, buffer_new);
  lua_pushnumber(L, total_length);
  lua_call(L, 1, 1);
  /* stack: target mtbl [total_length] list */
  target = (luv_buffer_t *)lua_touserdata(L, -1);
  dst = target->buf;

  for (i = 1; i <= n && total_length > 0;
      ++i, dst += len, total_length -= len) {
    lua_rawgeti(L, 1, i);
    buffer = (luv_buffer_t *)lua_touserdata(L, -1);
    /* stack: buf target mtbl [total_length] list */
    if (lua_getmetatable(L, -1) && lua_rawequal(L, -1, -4)) {
      /* stack: mtbl2 buf target mtbl [total_length] list */
      len = buffer->length;
      if (len > total_length)
        len = total_length;
      memmove(dst, buffer->buf, len);
      lua_pop(L, 2);
      /* stack: target mtbl [total_length] list */
    } else {
      return luaL_argerror(L, 1, "must be an array (table) of buffers");
    }
  }
  /* stack: target mtbl [total_length] list */
  lua_remove(L, -2);
  /* stack: target [total_length] list */
  return 1;
}

static const struct luaL_Reg buffer_functions[] = {
  { "concat", buffer_concat },
  { "isBuffer", buffer_is_buffer },
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
