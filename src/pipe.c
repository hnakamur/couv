#include "couv-private.h"

static uv_pipe_t *couv_alloc_pipe_handle(lua_State *L) {
  couv_pipe_t *w_handle;
  uv_pipe_t *handle;

  w_handle = couv_alloc(L, sizeof(couv_pipe_t));
  if (!w_handle)
    return NULL;

  handle = &w_handle->handle;

  if (couvL_is_mainthread(L)) {
    luaL_error(L, "pipe handle must be created in coroutine, not in main thread.");
    return NULL;
  } else
    couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));
  return handle;
}

void couv_free_pipe_handle(lua_State *L, uv_pipe_t *handle) {
  couv_pipe_t *w_handle;

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_THREAD_REG_KEY(handle));

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_LISTEN_CB_REG_KEY(handle));

  w_handle = container_of(handle, couv_pipe_t, handle);
  couv_free(L, w_handle);
}

static int pipe_create(lua_State *L) {
  uv_loop_t *loop;
  uv_pipe_t *handle;
  couv_stream_handle_data_t *hdata;
  int r;
  int ipc;

  ipc = lua_toboolean(L, 1);
  handle = couv_alloc_pipe_handle(L);
  if (!handle)
    return 0;

  loop = couv_loop(L);
  r = uv_pipe_init(loop, handle, ipc);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }

  handle->data = L;
  hdata = couv_get_stream_handle_data((uv_stream_t *)handle);
  hdata->is_yielded_for_input = 0;
  ngx_queue_init(&hdata->input_queue);

  lua_pushlightuserdata(L, handle);
  return 1;
}

static void connect_cb(uv_connect_t *req, int status) {
  lua_State *L;
  int nresults = 0;

  L = req->handle->data;
  if (status < 0) {
    lua_pushstring(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
    nresults = 1;
  }

  couv_free(L, req);
  couv_resume(L, L, nresults);
}

static int pipe_connect(lua_State *L) {
  uv_pipe_t *handle;
  uv_connect_t *req;
  const char *name;

  handle = lua_touserdata(L, 1);
  name = luaL_checkstring(L, 2);

  req = couv_alloc(L, sizeof(uv_connect_t));
  if (!req)
    return 0;
  uv_pipe_connect(req, handle, name, connect_cb);
  return lua_yield(L, 0);
}

static int pipe_bind(lua_State *L) {
  uv_pipe_t *handle;
  const char *name;
  int r;

  handle = lua_touserdata(L, 1);
  name = luaL_checkstring(L, 2);
  r = uv_pipe_bind(handle, name);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int pipe_open(lua_State *L) {
  uv_pipe_t *handle;
  int file;
  int r;

  handle = lua_touserdata(L, 1);
  file = luaL_checkint(L, 2);
  r = uv_pipe_open(handle, file);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static void read2_cb(uv_pipe_t *pipe, ssize_t nread, uv_buf_t buf,
    uv_handle_type pending) {
  couv_stream_handle_data_t *hdata;
  lua_State *L;
  couv_pipe_input_t *input;

  L = pipe->data;

  input = couv_alloc(L, sizeof(couv_pipe_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->w_buf.orig = buf.base;
  input->w_buf.buf = buf;
  input->pending = pending;
  hdata = couv_get_stream_handle_data((uv_stream_t *)pipe);
  ngx_queue_insert_tail(&hdata->input_queue, (ngx_queue_t *)input);

  if (lua_status(L) == LUA_YIELD && hdata->is_yielded_for_input) {
    hdata->is_yielded_for_input = 0;
    couv_resume(L, L, 0);
  }
}

static int couv_read2_start(lua_State *L) {
  uv_stream_t *handle;
  int r;

  handle = lua_touserdata(L, 1);
  r = uv_read2_start(handle, couv_buf_alloc_cb, read2_cb);
  if (r < 0) {
    luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));
  }
  return 0;
}

static int couv_prim_read2(lua_State *L) {
  uv_stream_t *handle;
  couv_stream_handle_data_t *hdata;
  couv_pipe_input_t *input;
  couv_buf_t *w_buf;

  handle = lua_touserdata(L, 1);
  hdata = couv_get_stream_handle_data(handle);

  if (ngx_queue_empty(&hdata->input_queue)) {
    hdata->is_yielded_for_input = 1;
    return lua_yield(L, 0);
  }
  input = (couv_pipe_input_t *)ngx_queue_head(&hdata->input_queue);
  ngx_queue_remove(input);

  lua_pushnumber(L, input->nread);

  w_buf = lua_newuserdata(L, sizeof(couv_buf_t));
  luaL_getmetatable(L, COUV_BUFFER_MTBL_NAME);
  lua_setmetatable(L, -2);
  *w_buf = input->w_buf;

  lua_pushnumber(L, input->pending);

  return 3;
}

static const struct luaL_Reg pipe_functions[] = {
  { "pipe_bind", pipe_bind },
  { "pipe_connect", pipe_connect },
  { "pipe_create", pipe_create },
  { "pipe_open", pipe_open },
  { "prim_read2", couv_prim_read2 },
  { "read2_start", couv_read2_start },
  { NULL, NULL }
};

int luaopen_couv_pipe(lua_State *L) {
  couvL_setfuncs(L, pipe_functions, 0);
  return 1;
}
