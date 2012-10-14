#include "yaluv-private.h"

static void connection_cb(uv_stream_t *handle, int status) {
  uv_loop_t *loop;
  lua_State *L;

  L = handle->data;
  loop = handle->loop;
printf("connection_cb loop=%lx, L=%lx\n", (unsigned long)loop, (unsigned long)L);
  if (status < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
    return;
  }

  luv_registry_get_for_ptr(L, ((char *)handle) + 1);
  luv_registry_get_for_ptr(L, handle);
  lua_call(L, 1, 0);
printf("connection_cb #2\n");

#if 0
/* TODO: move this to where connection close. __gc? */
  luv_registry_delete_for_ptr(L, ((char *)handle) + 1);
  luv_registry_delete_for_ptr(L, handle);
#endif
printf("connection_cb exit\n");
}

static int luv_listen(lua_State *L) {
  int r;
  uv_stream_t *handle;
  int backlog;

printf("listen L=%lx\n", (unsigned long)L);
  handle = lua_touserdata(L, 1);
  handle->data = L;
  backlog = luaL_checkint(L, 2);

  luaL_checktype(L, 3, LUA_TFUNCTION);

  luv_registry_set_for_ptr(L, handle, 1);
  luv_registry_set_for_ptr(L, ((char *)handle) + 1, 3);

  r = uv_listen(handle, backlog, connection_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("listen exit\n");
  return 0;
}

static int luv_accept(lua_State *L) {
  uv_stream_t *server;
  uv_stream_t *client;
  int r;

printf("accept L=%lx\n", (unsigned long)L);
  server = lua_touserdata(L, 1);
  client = lua_touserdata(L, 2);
  r = uv_accept(server, client);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("accept exit\n");
  return 0;
}

static uv_buf_t alloc_cb(uv_handle_t* handle, size_t suggested_size) {
  lua_State *L;
  void *p;

  L = (lua_State *)handle->data;
  p = luv_alloc(L, suggested_size);
  if (!p)
    return uv_buf_init(NULL, 0);
  return uv_buf_init(p, suggested_size);
}

static void read_cb(uv_stream_t* handle, ssize_t nread, uv_buf_t buf) {
  int r;
  uv_loop_t *loop;
  lua_State *L;
  luv_buf_t *lbuf;
  int ref;

  loop = handle->loop;
  L = (lua_State *)handle->data;
printf("read_cb L=%lx, nread=%ld\n", (unsigned long)L, nread);
  r = uv_read_stop(handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
    return;
  }

  if (nread == 0) {
    luv_resume(L, L, 0);
printf("read_cb exit nread=0\n");
    return;
  }

  lua_pushnumber(L, nread);

  lbuf = lua_newuserdata(L, sizeof(luv_buf_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  lbuf->orig = buf.base;
  lbuf->buf = buf;

printf("read_cb\n");
  luv_resume(L, L, 2);
}

static int luv_read_start(lua_State *L) {
  uv_stream_t *handle;
  int r;

printf("read_start L=%lx\n", (unsigned long)L);
  handle = lua_touserdata(L, 1);
  handle->data = L;
  r = uv_read_start(handle, alloc_cb, read_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("read_start exit\n");
  return lua_yield(L, 0);
}

static int luv_read_stop(lua_State *L) {
  uv_stream_t *handle;
  int r;

printf("read_stop L=%lx\n", (unsigned long)L);
  handle = lua_touserdata(L, 1);
  r = uv_read_stop(handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("read_stop\n");
  return 0;
}

static void write_cb(uv_write_t *req, int status) {
  lua_State *L;
  uv_stream_t *handle;

  handle = req->handle;
  L = handle->data;
printf("write_cb L=%lx\n", (unsigned long)L);

  if (status < 0) {
    luv_free(L, req->data);
    luv_free(L, req);
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
    return;
  }

  luv_free(L, req->data);
  luv_free(L, req);
printf("write_cb exit\n");
  luv_resume(L, L, 0);
}

static int luv_write(lua_State *L) {
  uv_write_t *req;
  uv_stream_t *handle;
  uv_buf_t *bufs;
  size_t bufcnt;
  int r;

printf("write L=%lx\n", (unsigned long)L);
  handle = (uv_stream_t *)lua_touserdata(L, 1);
  handle->data = L;
  bufs = luv_checkbuforstrtable(L, 2, &bufcnt);

  req = luv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;

  r = uv_write(req, handle, bufs, (int)bufcnt, write_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("write exit\n");
  return lua_yield(L, 0);
}


static void close_cb(uv_handle_t *handle) {
  lua_State *L = (lua_State *)handle->data;
  luv_resume(L, L, 0);
}

static int luv_close(lua_State *L) {
  uv_handle_t *handle = lua_touserdata(L, 1);
  handle->data = L;
  uv_close(handle, close_cb);
  return lua_yield(L, 0);
}

static const struct luaL_Reg functions[] = {
  { "accept", luv_accept },
  { "listen", luv_listen },
  { "close", luv_close },
  { "read_start", luv_read_start },
  { "read_stop", luv_read_stop },
  { "write", luv_write },
  { NULL, NULL }
};

int luaopen_yaluv(lua_State *L) {
printf("luaopen_yaluv L=%lx\n", (unsigned long)L);
  luaL_register(L, "yaluv", functions);

  luaopen_yaluv_loop(L);

  luaopen_yaluv_buffer(L);
  luaopen_yaluv_fs(L);
  luaopen_yaluv_ipaddr(L);
  luaopen_yaluv_tcp(L);
  luaopen_yaluv_udp(L);

  return 1;
}
