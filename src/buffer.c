#include "couv-private.h"

#define FLOAT_SIZE ((int)sizeof(float))
#define DOUBLE_SIZE ((int)sizeof(double))

void *couv_buf_mem_alloc(lua_State *L, size_t nbytes) {
  couv_buf_mem_t *mem;

  mem = couv_alloc(L, sizeof(int) + sizeof(couv_free_t) + nbytes);
  if (!mem)
    return NULL;

  mem->ref_cnt = 1;
  mem->free = couv_free;
  return mem->mem;
}

void couv_buf_mem_retain(lua_State *L, void *ptr) {
  couv_buf_mem_t *mem;

  mem = container_of(ptr, couv_buf_mem_t, mem);
  ++mem->ref_cnt;
}

void couv_buf_mem_release(lua_State *L, void *ptr) {
  couv_buf_mem_t *mem;

  mem = container_of(ptr, couv_buf_mem_t, mem);
  if (!--mem->ref_cnt) {
    mem->free(L, mem);
  }
}


uv_buf_t couv_tobuforstr(lua_State *L, int index) {
  uv_buf_t buf;
  couv_buf_t *lbuf;
  int type = lua_type(L, index);
  if (type == LUA_TSTRING) {
    buf.base = (char *)lua_tolstring(L, index, &buf.len);
    return buf;
  } else if (couvL_hasmetatablename(L, index, COUV_BUFFER_MTBL_NAME)) {
    lbuf = lua_touserdata(L, index);
    return lbuf->buf;
  } else {
    buf.base = NULL;
    buf.len = 0;
    return buf;
  }
}

uv_buf_t couv_checkbuforstr(lua_State *L, int index) {
  uv_buf_t buf = couv_tobuforstr(L, index);
  if (!buf.base)
    luaL_argerror(L, index, "must be string or Buffer");
  return buf;
}

uv_buf_t *couv_checkbuforstrtable(lua_State *L, int index, size_t *bufcnt) {
  int i;
  int n;
  uv_buf_t buf;
  uv_buf_t *buffers;

  luaL_checktype(L, index, LUA_TTABLE);
  n = lua_objlen(L, index);
  buffers = couv_alloc(L, n * sizeof(uv_buf_t));
  if (!buffers)
    return NULL;

  for (i = 1; i <= n; ++i) {
    lua_rawgeti(L, index, i);
    buf = couv_tobuforstr(L, -1);
    lua_pop(L, 1);
    if (buf.base) {
      buffers[i - 1] = buf;
    } else {
      couv_free(L, buffers);
      luaL_argerror(L, 1, "must be an array (table) of buffers");
      return NULL;
    }
  }
  *bufcnt = n;
  return buffers;
}

void couv_dbg_print_bufs(const char *header, uv_buf_t *bufs, size_t bufcnt) {
  size_t i;

  for (i = 0; i < bufcnt; ++i) {
    printf("%s bufs[%lu] len=%ld, base=%s\n", header, i, bufs[i].len,
        bufs[i].base);
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
  couv_buf_t *buf = couv_checkbuf(L, 1);
  if (buf->orig)
    couv_buf_mem_release(L, buf->orig);
  return 0;
}

static int buffer_read_uint8(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len);

  lua_pushnumber(L, *(unsigned char *)&buf->buf.base[position - 1]);
  return 1;
}

static int buffer_read_uint16le(lua_State *L) {
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  p = (unsigned char *)&buf->buf.base[position - 1];
  lua_pushnumber(L, p[1] << 8 | p[0]);
  return 1;
}

static int buffer_read_uint16be(lua_State *L) {
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  p = (unsigned char *)&buf->buf.base[position - 1];
  lua_pushnumber(L, p[0] << 8 | p[1]);
  return 1;
}

static int buffer_read_uint32le(lua_State *L) {
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  p = (unsigned char *)&buf->buf.base[position - 1];
  lua_pushnumber(L, (unsigned)p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0]);
  return 1;
}

static int buffer_read_uint32be(lua_State *L) {
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  p = (unsigned char *)&buf->buf.base[position - 1];
  lua_pushnumber(L, (unsigned)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]);
  return 1;
}

static int buffer_read_int8(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len);

  lua_pushnumber(L, buf->buf.base[position - 1]);
  return 1;
}

static int buffer_read_int16le(lua_State *L) {
  int value;
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  p = (unsigned char *)&buf->buf.base[position - 1];
  value = p[1] << 8 | p[0];
  if (value >= 0x8000)
    value = -(0xFFFF - value + 1);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_int16be(lua_State *L) {
  int value;
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  p = (unsigned char *)&buf->buf.base[position - 1];
  value = p[0] << 8 | p[1];
  if (value >= 0x8000)
    value = -(0xFFFF - value + 1);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_int32le(lua_State *L) {
  unsigned int value;
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  p = (unsigned char *)&buf->buf.base[position - 1];
  value = p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0];
  lua_pushnumber(L, value >= 0x80000000
      ? -(int)(0xFFFFFFFF - value + 1) : (int)value);
  return 1;
}

static int buffer_read_int32be(lua_State *L) {
  unsigned int value;
  unsigned char *p;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  p = (unsigned char *)&buf->buf.base[position - 1];
  value = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
  lua_pushnumber(L, value >= 0x80000000 ?
      -(int)(0xFFFFFFFF - value + 1) : (int)value);
  return 1;
}

static int buffer_read_float_le(lua_State *L) {
  float value;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (FLOAT_SIZE - 1));

  memcpy_le(&buf->buf.base[position - 1], (char *)&value, FLOAT_SIZE);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_float_be(lua_State *L) {
  float value;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (FLOAT_SIZE - 1));

  memcpy_be(&buf->buf.base[position - 1], (char *)&value, FLOAT_SIZE);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_double_le(lua_State *L) {
  double value;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (DOUBLE_SIZE - 1));

  memcpy_le(&buf->buf.base[position - 1], (char *)&value, DOUBLE_SIZE);
  lua_pushnumber(L, value);
  return 1;
}

static int buffer_read_double_be(lua_State *L) {
  double value;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (DOUBLE_SIZE - 1));

  memcpy_be(&buf->buf.base[position - 1], (char *)&value, DOUBLE_SIZE);
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
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  int byte = luaL_checkint(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len);

  *(unsigned char *)&buf->buf.base[position - 1] = (unsigned char)byte;
  return 0;
}

static int buffer_write_uint16le(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = value & 0xFF;
  *q++ = value >> 8;
  return 0;
}

static int buffer_write_uint16be(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = value >> 8;
  *q++ = value & 0xFF;
  return 0;
}

static int buffer_write_uint32le(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = value & 0xFF;
  *q++ = (value >> 8) & 0xFF;
  *q++ = (value >> 16) & 0xFF;
  *q++ = value >> 24;
  return 0;
}

static int buffer_write_uint32be(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  unsigned int value = (unsigned int)luaL_checkinteger(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = value >> 24;
  *q++ = (value >> 16) & 0xFF;
  *q++ = (value >> 8) & 0xFF;
  *q++ = value & 0xFF;
  return 0;
}

static int buffer_write_int8(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  int byte = luaL_checkint(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len);

  buf->buf.base[position - 1] = (char)byte;
  return 0;
}

static int buffer_write_int16le(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x10000 + value : value;
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = uvalue & 0xFF;
  *q++ = uvalue >> 8;
  return 0;
}

static int buffer_write_int16be(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x10000 + value : value;
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 1);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = uvalue >> 8;
  *q++ = uvalue & 0xFF;
  return 0;
}

static int buffer_write_int32le(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x100000000 + value : value;
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = uvalue & 0xFF;
  *q++ = (uvalue >> 8) & 0xFF;
  *q++ = (uvalue >> 16) & 0xFF;
  *q++ = uvalue >> 24;
  return 0;
}

static int buffer_write_int32be(lua_State *L) {
  unsigned char *q;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  int value = luaL_checkinteger(L, 3);
  unsigned int uvalue = value < 0 ? 0x100000000 + value : value;
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - 3);

  q = (unsigned char *)&buf->buf.base[position - 1];
  *q++ = uvalue >> 24;
  *q++ = (uvalue >> 16) & 0xFF;
  *q++ = (uvalue >> 8) & 0xFF;
  *q++ = uvalue & 0xFF;
  return 0;
}

static int buffer_write_float_le(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  float value = (float)luaL_checknumber(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (FLOAT_SIZE - 1));

  memcpy_le((char *)&value, &buf->buf.base[position - 1], FLOAT_SIZE);
  return 0;
}

static int buffer_write_float_be(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  float value = (float)luaL_checknumber(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (FLOAT_SIZE - 1));

  memcpy_be((char *)&value, &buf->buf.base[position - 1], FLOAT_SIZE);
  return 0;
}

static int buffer_write_double_le(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  double value = (double)luaL_checknumber(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (DOUBLE_SIZE - 1));

  memcpy_le((char *)&value, &buf->buf.base[position - 1], DOUBLE_SIZE);
  return 0;
}

static int buffer_write_double_be(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int position = luaL_checkint(L, 2);
  double value = (double)luaL_checknumber(L, 3);
  couv_argcheckindex(L, 2, position, 1, buf->buf.len - (DOUBLE_SIZE - 1));

  memcpy_be((char *)&value, &buf->buf.base[position - 1], DOUBLE_SIZE);
  return 0;
}

static int buffer_length(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  lua_pushnumber(L, buf->buf.len);
  return 1;
}

static int buffer_slice(lua_State *L) {
  couv_buf_t *slice;
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int first = luaL_optint(L, 2, 1);
  int last = luaL_optint(L, 3, buf->buf.len);
  couv_argcheckindex(L, 2, first, 1, buf->buf.len);
  couv_argcheckindex(L, 3, last, first, buf->buf.len);

  slice = (couv_buf_t *)lua_newuserdata(L, sizeof(couv_buf_t));
  luaL_getmetatable(L, COUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);

  couv_buf_mem_retain(L, buf->orig);
  slice->orig = buf->orig;
  slice->buf.len = last - first + 1;
  slice->buf.base = buf->buf.base + first - 1;

  return 1;
}

static int buffer_fill(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int byte = luaL_checkint(L, 2);
  int first = luaL_optint(L, 3, 1);
  int last = luaL_optint(L, 4, buf->buf.len);
  couv_argcheckindex(L, 3, first, 1, buf->buf.len);
  couv_argcheckindex(L, 4, last, first, buf->buf.len);

  memset(buf->buf.base + first - 1, byte, last - first + 1);
  return 0;
}

static int buffer_to_string(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  int first = luaL_optint(L, 2, 1);
  int last = luaL_optint(L, 3, buf->buf.len);
  couv_argcheckindex(L, 2, first, 1, buf->buf.len);
  couv_argcheckindex(L, 3, last, first, buf->buf.len);

  lua_pushlstring(L, buf->buf.base + first - 1, last - first + 1);
  return 1;
}

static int buffer_copy(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  couv_buf_t *target = couv_checkbuf(L, 2);
  int target_first = luaL_optint(L, 3, 1);
  int source_first = luaL_optint(L, 4, 1);
  int source_last = luaL_optint(L, 5, buf->buf.len);
  int length;
  couv_argcheckindex(L, 3, target_first, 1, target->buf.len);
  couv_argcheckindex(L, 4, source_first, 1, buf->buf.len);
  couv_argcheckindex(L, 5, source_last, source_first, buf->buf.len);

  length = source_last - source_first + 1;
  if (length > (int)target->buf.len - target_first + 1)
    length = (int)target->buf.len - target_first + 1;
  if (length > 0)
    memmove(&target->buf.base[target_first - 1],
        &buf->buf.base[source_first - 1], length);
  lua_pushnumber(L, length);
  return 1;
}

static int buffer_write(lua_State *L) {
  couv_buf_t *buf = couv_checkbuf(L, 1);
  size_t str_len;
  const char *str = luaL_checklstring(L, 2, &str_len);
  int position = luaL_optint(L, 3, 1);
  int length = luaL_optint(L, 4, buf->buf.len - position + 1);
  couv_argcheckindex(L, 3, position, 1, buf->buf.len);
  couv_argcheckindex(L, 4, length, 0, buf->buf.len - position + 1);

  if (length > (int)str_len)
    length = (int)str_len;
  if (length > 0)
    memcpy(&buf->buf.base[position - 1], str, length);
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
  lua_pushboolean(L, couvL_hasmetatablename(L, 1, COUV_BUFFER_MTBL_NAME));
  return 1;
}

static int buffer_new(lua_State *L) {
  couv_buf_t *buf;
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

  buf = lua_newuserdata(L, sizeof(couv_buf_t));
  if (!buf)
    return 0;
  luaL_getmetatable(L, COUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  buf->orig = buf->buf.base = couv_buf_mem_alloc(L, length);
  if (!buf->orig)
    return 0;
  buf->buf.len = length;
  if (str)
    memcpy(buf->buf.base, str, length);

  return 1;
}

static int buffer_concat(lua_State *L) {
  size_t i;
  size_t n;
  int total_length;
  uv_buf_t *buf;
  uv_buf_t *buffers;
  couv_buf_t *target;
  char *dst;
  int len;

  buffers = couv_checkbuforstrtable(L, 1, &n);
  total_length = luaL_optint(L, 2, 0);

  if (total_length == 0) {
    for (i = 0; i < n; ++i) {
      buf = &buffers[i];
      total_length += buf->len;
    }
  }

  lua_pushcfunction(L, buffer_new);
  lua_pushnumber(L, total_length);
  lua_call(L, 1, 1);
  /* stack: target [total_length] list */
  target = (couv_buf_t *)lua_touserdata(L, -1);
  dst = target->buf.base;

  for (i = 0; i < n && total_length > 0; ++i, dst += len, total_length -= len) {
    buf = &buffers[i];
    len = buf->len;
    if (len > total_length)
      len = total_length;
    memmove(dst, buf->base, len);
  }
  free(buffers);
  return 1;
}

static const struct luaL_Reg buffer_functions[] = {
  { "concat", buffer_concat },
  { "isBuffer", buffer_is_buffer },
  { "new", buffer_new },
  { NULL, NULL }
};

int luaopen_couv_buffer(lua_State *L) {
  luaL_newmetatable(L, COUV_BUFFER_MTBL_NAME);
  luaL_register(L, NULL, buffer_methods);
  lua_pop(L, 1);

  lua_createtable(L, 0, ARRAY_SIZE(buffer_functions) - 1);
  luaL_register(L, NULL, buffer_functions);
  lua_setfield(L, -2, "Buffer");
  return 1;
}
