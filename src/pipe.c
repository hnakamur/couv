#include "couv-private.h"

static uv_pipe_t *couv_alloc_pipe_handle(lua_State *L) {
  couv_pipe_t *w_handle;

  w_handle = couv_alloc(L, sizeof(couv_pipe_t));
  if (!w_handle)
    return NULL;

  if (couvL_is_mainthread(L)) {
    luaL_error(L, "pipe handle must be created in coroutine, not in main thread.");
    return NULL;
  } else
    w_handle->threadref = luaL_ref(L, LUA_REGISTRYINDEX);
  return &w_handle->handle;
}

void couv_free_pipe_handle(lua_State *L, uv_pipe_t *handle) {
  couv_pipe_t *w_handle;

  w_handle = container_of(handle, couv_pipe_t, handle);
  luaL_unref(L, LUA_REGISTRYINDEX, w_handle->threadref);
  couv_free(L, w_handle);
}

static int pipe_create(lua_State *L) {
  uv_loop_t *loop;
  uv_pipe_t *handle;
  couv_pipe_t *w_handle;
  int r;
  int ipc;

  ipc = lua_toboolean(L, 1);
  handle = couv_alloc_pipe_handle(L);
  if (!handle)
    return 0;

  w_handle = container_of(handle, couv_pipe_t, handle);
  w_handle->is_yielded_for_read = 0;
  ngx_queue_init(&w_handle->input_queue);
  loop = couv_loop(L);
  r = uv_pipe_init(loop, handle, ipc);
  if (r < 0) {
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  }
  handle->data = L;
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

static const struct luaL_Reg pipe_functions[] = {
  { "pipe_bind", pipe_bind },
  { "pipe_connect", pipe_connect },
  { "pipe_create", pipe_create },
  { "pipe_open", pipe_open },
  { NULL, NULL }
};

int luaopen_couv_pipe(lua_State *L) {
  couvL_setfuncs(L, pipe_functions, 0);
  return 1;
}
