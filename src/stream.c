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

static int couv_get_write_queue_size(lua_State *L) {
  uv_stream_t *handle;

  handle = lua_touserdata(L, 1);
  lua_pushnumber(L, handle->write_queue_size);
  return 1;
}

static int couv_is_readable(lua_State *L) {
  uv_stream_t *handle;

  handle = lua_touserdata(L, 1);
  lua_pushboolean(L, uv_is_readable(handle));
  return 1;
}

static int couv_is_writable(lua_State *L) {
  uv_stream_t *handle;

  handle = lua_touserdata(L, 1);
  lua_pushboolean(L, uv_is_writable(handle));
  return 1;
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


static void shutdown_cb(uv_shutdown_t *req, int status) {
  lua_State *L;
  uv_stream_t *handle;

  handle = req->handle;
  L = handle->data;

  if (status < 0) {
    couv_free(L, req);
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    return;
  }

  couv_free(L, req);
  couv_resume(L, L, 0);
}

static int couv_shutdown(lua_State *L) {
  uv_stream_t *handle;
  uv_shutdown_t* req;
  int r;

  handle = lua_touserdata(L, 1);
  req = couv_alloc(L, sizeof(uv_shutdown_t));
  r = uv_shutdown(req, handle, shutdown_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
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

static int couv_write2(lua_State *L) {
  uv_write_t *req;
  uv_stream_t *handle;
  uv_buf_t *bufs;
  size_t bufcnt;
  uv_stream_t *send_handle;
  int r;

  handle = lua_touserdata(L, 1);
  bufs = couv_checkbuforstrtable(L, 2, &bufcnt);
  send_handle = lua_touserdata(L, 3);

  req = couv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;

  r = uv_write2(req, handle, bufs, (int)bufcnt, send_handle, write_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}

static const struct luaL_Reg stream_functions[] = {
  { "accept", couv_accept },
  { "get_write_queue_size", couv_get_write_queue_size },
  { "is_readable", couv_is_readable },
  { "is_writable", couv_is_writable },
  { "listen", couv_listen },
  { "prim_read", couv_prim_read },
  { "read_start", couv_read_start },
  { "read_stop", couv_read_stop },
  { "shutdown", couv_shutdown },
  { "write", couv_write },
  { "write2", couv_write2 },
  { NULL, NULL }
};

int luaopen_couv_stream(lua_State *L) {
  couvL_setfuncs(L, stream_functions, 0);
  return 1;
}
