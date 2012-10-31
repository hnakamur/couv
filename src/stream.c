#include "couv-private.h"

couv_stream_handle_data_t *couv_get_stream_handle_data(uv_stream_t *handle) {
  switch (handle->type) {
  case UV_TCP:
    return &((couv_tcp_t *)handle)->hdata;
  case UV_NAMED_PIPE:
    return &((couv_pipe_t *)handle)->hdata;
  case UV_TTY:
    return &((couv_tty_t *)handle)->hdata;
  default:
    return NULL;
  }
}

static void connection_cb(uv_stream_t *handle, int status) {
  lua_State *L;

  L = handle->data;
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_LISTEN_CB_REG_KEY(handle));
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
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

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  backlog = luaL_checkint(L, 2);
  luaL_checktype(L, 3, LUA_TFUNCTION);
  lua_pushvalue(L, 3);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_LISTEN_CB_REG_KEY(handle));

  r = uv_listen(handle, backlog, connection_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  lua_pushvalue(L, 1);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
  return 0;
}

static int couv_accept(lua_State *L) {
  uv_stream_t *server;
  uv_stream_t *client;
  int r;

  server = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  client = couvL_checkudataclass(L, 2, COUV_STREAM_MTBL_NAME);
  r = uv_accept(server, client);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int couv_get_write_queue_size(lua_State *L) {
  uv_stream_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  lua_pushnumber(L, handle->write_queue_size);
  return 1;
}

static int couv_is_readable(lua_State *L) {
  uv_stream_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  lua_pushboolean(L, uv_is_readable(handle));
  return 1;
}

static int couv_is_writable(lua_State *L) {
  uv_stream_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  lua_pushboolean(L, uv_is_writable(handle));
  return 1;
}

static void read_cb(uv_stream_t *handle, ssize_t nread, uv_buf_t buf) {
  lua_State *L;
  couv_stream_input_t *input;
  couv_stream_handle_data_t *hdata;

  L = handle->data;

  input = couv_alloc(L, sizeof(couv_stream_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->w_buf.orig = buf.base;
  input->w_buf.buf = buf;
  hdata = couv_get_stream_handle_data(handle);
  ngx_queue_insert_tail(&hdata->input_queue, (ngx_queue_t *)input);

  if (lua_status(L) == LUA_YIELD && hdata->is_yielded_for_input) {
    hdata->is_yielded_for_input = 0;
    couv_resume(L, L, 0);
  }
}

static int couv_read_start(lua_State *L) {
  uv_stream_t *handle;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  r = uv_read_start(handle, couv_buf_alloc_cb, read_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int couv_prim_read(lua_State *L) {
  uv_stream_t *handle;
  couv_stream_input_t *input;
  couv_stream_handle_data_t *hdata;
  couv_buf_t *w_buf;

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);

  hdata = couv_get_stream_handle_data(handle);
  if (ngx_queue_empty(&hdata->input_queue)) {
    hdata->is_yielded_for_input = 1;
    return lua_yield(L, 0);
  }
  input = (couv_stream_input_t *)ngx_queue_head(&hdata->input_queue);
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

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
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

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
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
  int nargs;

  handle = req->handle;
  L = handle->data;

  couv_free(L, req->data);
  couv_free(L, req);

#if 1
printf("write_cb L=%lx\n", (unsigned long)L);
printf("write_cb req=%lx, status=%d\n", (unsigned long)req, status);
  lua_pushlightuserdata(L, handle);
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_WRITE_CB_REG_KEY(handle));
printf("write_cb write_cb=%lx\n", (unsigned long)lua_topointer(L, -1));
  lua_pushlightuserdata(L, handle);
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));
printf("write_cb thread=%lx\n", (unsigned long)lua_topointer(L, -1));
  if (status < 0) {
    lua_pushstring(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    nargs = 2;
  } else
    nargs = 1;
printf("write_cb before lua_call nargs=%d\n", nargs);
  lua_call(L, nargs, 0);
#else
  if (status < 0)
    lua_pushstring(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  couv_resume(L, L, 0);
#endif
}

static int couv_write(lua_State *L) {
  uv_write_t *req;
  uv_stream_t *handle;
  uv_buf_t *bufs;
  size_t bufcnt;
  int r;

printf("write L=%lx\n", (unsigned long)L);
  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  bufs = couv_checkbuforstrtable(L, 2, &bufcnt);

  luaL_checktype(L, 3, LUA_TFUNCTION);
  lua_pushvalue(L, 3);
printf("write write_cb=%lx\n", (unsigned long)lua_topointer(L, -1));
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_WRITE_CB_REG_KEY(handle));

  req = couv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;
printf("write req=%lx\n", (unsigned long)req);

  lua_pushvalue(L, 1);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));

  r = uv_write(req, handle, bufs, (int)bufcnt, write_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
#if 1
  return 0;
#else
  return lua_yield(L, 0);
#endif
}

static int couv_write2(lua_State *L) {
  uv_write_t *req;
  uv_stream_t *handle;
  uv_buf_t *bufs;
  size_t bufcnt;
  uv_stream_t *send_handle;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_STREAM_MTBL_NAME);
  bufs = couv_checkbuforstrtable(L, 2, &bufcnt);
  send_handle = couvL_checkudataclass(L, 3, COUV_STREAM_MTBL_NAME);

  req = couv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;

  r = uv_write2(req, handle, bufs, (int)bufcnt, send_handle, write_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return lua_yield(L, 0);
}

static const struct luaL_Reg stream_methods[] = {
  { "accept", couv_accept },
  { "getWriteQueueSize", couv_get_write_queue_size },
  { "isReadable", couv_is_readable },
  { "isWritable", couv_is_writable },
  { "listen", couv_listen },
  { "_read", couv_prim_read },
  { "_shutdown", couv_shutdown },
  { "startRead", couv_read_start },
  { "stopRead", couv_read_stop },
  { "_write", couv_write },
  { "write2", couv_write2 },
  { NULL, NULL }
};

static const struct luaL_Reg stream_functions[] = {
  { NULL, NULL }
};

int luaopen_couv_stream(lua_State *L) {
  lua_newtable(L);
  couvL_setfuncs(L, stream_functions, 0);
  lua_setfield(L, -2, "Stream");

  couv_newmetatable(L, COUV_STREAM_MTBL_NAME, COUV_HANDLE_MTBL_NAME);
  couvL_setfuncs(L, stream_methods, 0);
  lua_setfield(L, -2, "_Stream");

  return 0;
}
