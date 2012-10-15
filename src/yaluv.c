#include "yaluv-private.h"

static void connection_cb(uv_stream_t *handle, int status) {
  uv_loop_t *loop;
  lua_State *L;
  luv_stream_t *lhandle;

  L = handle->data;
  loop = handle->loop;
  lhandle = container_of(handle, luv_stream_t, handle);
  luv_registry_get_for_ptr(L, ((char *)lhandle) + 1);
  luv_registry_get_for_ptr(L, lhandle);
  if (status < 0) {
    lua_pushstring(L, luvL_uv_errname(uv_last_error(loop).code));
    lua_call(L, 2, 0);
  } else {
    lua_call(L, 1, 0);
  }

#if 0
/* TODO: move this to where connection close. __gc? */
  luv_registry_delete_for_ptr(L, ((char *)handle) + 1);
  luv_registry_delete_for_ptr(L, handle);
#endif
}

static int luv_listen(lua_State *L) {
  int r;
  luv_stream_t *lhandle;
  int backlog;

  lhandle = lua_touserdata(L, 1);
  backlog = luaL_checkint(L, 2);

  luaL_checktype(L, 3, LUA_TFUNCTION);

  luv_registry_set_for_ptr(L, lhandle, 1);
  luv_registry_set_for_ptr(L, ((char *)lhandle) + 1, 3);

  r = uv_listen(&lhandle->handle, backlog, connection_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

static int luv_accept(lua_State *L) {
  luv_stream_t *server;
  luv_stream_t *client;
  int r;

  server = lua_touserdata(L, 1);
  client = lua_touserdata(L, 2);
  r = uv_accept(&server->handle, &client->handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

static uv_buf_t alloc_cb(uv_handle_t *handle, size_t suggested_size) {
  lua_State *L;
  void *p;

  L = handle->data;
  p = luv_buf_mem_alloc(L, suggested_size);
  if (!p)
    return uv_buf_init(NULL, 0);
  return uv_buf_init(p, suggested_size);
}

static void read_cb(uv_stream_t *handle, ssize_t nread, uv_buf_t buf) {
  luv_stream_t *lhandle;
  lua_State *L;
  luv_stream_input_t *input;

  L = handle->data;
  lhandle = container_of(handle, luv_stream_t, handle);

  input = luv_alloc(L, sizeof(luv_stream_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->lbuf.orig = buf.base;
  input->lbuf.buf = buf;
  ngx_queue_insert_tail(&lhandle->input_queue, (ngx_queue_t *)input);

  if (lua_status(L) == LUA_YIELD && lhandle->is_yielded_for_read) {
    lhandle->is_yielded_for_read = 0;
    luv_resume(L, L, 0);
  }
}

static int luv_read_start(lua_State *L) {
  luv_stream_t *lhandle;
  int r;

  lhandle = lua_touserdata(L, 1);
  r = uv_read_start(&lhandle->handle, alloc_cb, read_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

static int luv_read_stop(lua_State *L) {
  luv_stream_t *lhandle;
  int r;

  lhandle = lua_touserdata(L, 1);
  r = uv_read_stop(&lhandle->handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return 0;
}

static int luv_prim_read(lua_State *L) {
  luv_stream_t *lhandle;
  luv_stream_input_t *input;
  luv_buf_t *lbuf;

  lhandle = lua_touserdata(L, 1);

  if (ngx_queue_empty(&lhandle->input_queue)) {
    lhandle->is_yielded_for_read = 1;
    return lua_yield(L, 0);
  }
  input = (luv_stream_input_t *)ngx_queue_head(&lhandle->input_queue);
  ngx_queue_remove(input);

  lua_pushnumber(L, input->nread);

  lbuf = lua_newuserdata(L, sizeof(luv_buf_t));
  luaL_getmetatable(L, LUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  *lbuf = input->lbuf;

  return 2;
}

static void write_cb(uv_write_t *req, int status) {
  lua_State *L;
  uv_stream_t *handle;

  handle = req->handle;
  L = handle->data;

  if (status < 0) {
    luv_free(L, req->data);
    luv_free(L, req);
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
    return;
  }

  luv_free(L, req->data);
  luv_free(L, req);
  luv_resume(L, L, 0);
}

static int luv_write(lua_State *L) {
  uv_write_t *req;
  luv_stream_t *lhandle;
  uv_buf_t *bufs;
  size_t bufcnt;
  int r;

  lhandle = lua_touserdata(L, 1);
  bufs = luv_checkbuforstrtable(L, 2, &bufcnt);

  req = luv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;

  r = uv_write(req, &lhandle->handle, bufs, (int)bufcnt, write_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
  return lua_yield(L, 0);
}


static void close_cb(uv_handle_t *handle) {
  lua_State *L;

  L = handle->data;
  luv_resume(L, L, 0);
}

static int luv_close(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  uv_close(handle, close_cb);
  return lua_yield(L, 0);
}

static const struct luaL_Reg functions[] = {
  { "accept", luv_accept },
  { "listen", luv_listen },
  { "close", luv_close },
  { "prim_read", luv_prim_read },
  { "read_start", luv_read_start },
  { "read_stop", luv_read_stop },
  { "write", luv_write },
  { NULL, NULL }
};

int luaopen_yaluv_native(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(functions) - 1);
  luaL_register(L, NULL, functions);

  luaopen_yaluv_loop(L);

  luaopen_yaluv_buffer(L);
  luaopen_yaluv_fs(L);
  luaopen_yaluv_ipaddr(L);
  luaopen_yaluv_tcp(L);
  luaopen_yaluv_udp(L);

  return 1;
}
