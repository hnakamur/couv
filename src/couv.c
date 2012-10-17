#include "couv-private.h"

static void connection_cb(uv_stream_t *handle, int status) {
  lua_State *L;

  L = handle->data;
  couv_registry_get_for_ptr(L, handle);
  lua_pushlightuserdata(L, handle);
  if (status < 0) {
    lua_pushstring(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    lua_call(L, 2, 0);
  } else {
    lua_call(L, 1, 0);
  }
}

static int couv_listen(lua_State *L) {
  int r;
  uv_stream_t *handle;
  int backlog;

  handle = lua_touserdata(L, 1);
  backlog = luaL_checkint(L, 2);
  luaL_checktype(L, 3, LUA_TFUNCTION);
  couv_registry_set_for_ptr(L, handle, 3);

  r = uv_listen(handle, backlog, connection_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int couv_accept(lua_State *L) {
  uv_stream_t *server;
  uv_stream_t *client;
  int r;

  server = lua_touserdata(L, 1);
  client = lua_touserdata(L, 2);
  r = uv_accept(server, client);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static void read_cb(uv_stream_t *handle, ssize_t nread, uv_buf_t buf) {
  couv_stream_t *w_handle;
  lua_State *L;
  couv_stream_input_t *input;

  L = handle->data;
  w_handle = container_of(handle, couv_stream_t, handle);

  input = couv_alloc(L, sizeof(couv_stream_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->w_buf.orig = buf.base;
  input->w_buf.buf = buf;
  ngx_queue_insert_tail(&w_handle->input_queue, (ngx_queue_t *)input);

  if (lua_status(L) == LUA_YIELD && w_handle->is_yielded_for_read) {
    w_handle->is_yielded_for_read = 0;
    couv_resume(L, L, 0);
  }
}

static int couv_read_start(lua_State *L) {
  uv_stream_t *handle;
  int r;

  handle = lua_touserdata(L, 1);
  r = uv_read_start(handle, couv_buf_alloc_cb, read_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int couv_read_stop(lua_State *L) {
  uv_stream_t *handle;
  int r;

  handle = lua_touserdata(L, 1);
  r = uv_read_stop(handle);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int couv_prim_read(lua_State *L) {
  uv_stream_t *handle;
  couv_stream_t *w_handle;
  couv_stream_input_t *input;
  couv_buf_t *w_buf;

  handle = lua_touserdata(L, 1);
  w_handle = container_of(handle, couv_stream_t, handle);

  if (ngx_queue_empty(&w_handle->input_queue)) {
    w_handle->is_yielded_for_read = 1;
    return lua_yield(L, 0);
  }
  input = (couv_stream_input_t *)ngx_queue_head(&w_handle->input_queue);
  ngx_queue_remove(input);

  lua_pushnumber(L, input->nread);

  w_buf = lua_newuserdata(L, sizeof(couv_buf_t));
  luaL_getmetatable(L, COUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  *w_buf = input->w_buf;

  return 2;
}

static void write_cb(uv_write_t *req, int status) {
  lua_State *L;
  uv_stream_t *handle;

  handle = req->handle;
  L = handle->data;

  if (status < 0) {
    couv_free(L, req->data);
    couv_free(L, req);
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    return;
  }

  couv_free(L, req->data);
  couv_free(L, req);
  couv_resume(L, L, 0);
}

static int couv_write(lua_State *L) {
  uv_write_t *req;
  uv_stream_t *handle;
  uv_buf_t *bufs;
  size_t bufcnt;
  int r;

  handle = lua_touserdata(L, 1);
  bufs = couv_checkbuforstrtable(L, 2, &bufcnt);

  req = couv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;

  r = uv_write(req, handle, bufs, (int)bufcnt, write_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}


static void close_cb(uv_handle_t *handle) {
  lua_State *L;

  L = handle->data;

  switch (handle->type) {
  case UV_TCP:
    couv_free_tcp_handle(L, (uv_tcp_t *)handle);
    break;
  case UV_UDP:
    couv_free_udp_handle(L, (uv_udp_t *)handle);
    break;
  default:
    /* do nothing */
    break;
  }

  /* If we close handle from another thread, the thread for handle is not
   * yielded, so no need to resume.
   */
  if (lua_status(L) == LUA_YIELD)
    couv_resume(L, L, 0);
}

static int couv_close(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
  uv_close(handle, close_cb);
  return lua_yield(L, 0);
}

static int couv_hrtime(lua_State *L) {
  lua_pushnumber(L, uv_hrtime() / 1e9);
  return 1;
}

static const struct luaL_Reg functions[] = {
  { "accept", couv_accept },
  { "listen", couv_listen },
  { "close", couv_close },
  { "hrtime", couv_hrtime },
  { "prim_read", couv_prim_read },
  { "read_start", couv_read_start },
  { "read_stop", couv_read_stop },
  { "write", couv_write },
  { NULL, NULL }
};

int luaopen_couv_native(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(functions) - 1);
  luaL_register(L, NULL, functions);

  luaopen_couv_loop(L);

  luaopen_couv_buffer(L);
  luaopen_couv_fs(L);
  luaopen_couv_ipaddr(L);
  luaopen_couv_tcp(L);
  luaopen_couv_udp(L);

  return 1;
}
