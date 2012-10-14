#include "yaluv-private.h"

static void connection_cb(uv_stream_t *handle, int status) {
  uv_loop_t *loop;
  lua_State *L;
  luv_stream_t *lhandle;

  L = handle->data;
  loop = handle->loop;
printf("connection_cb#1 loop=%lx, L=%lx\n", (unsigned long)loop, (unsigned long)L);
  lhandle = container_of(handle, luv_stream_t, handle);
  if (status < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(loop).code));
    return;
  }

  luv_registry_get_for_ptr(L, ((char *)lhandle) + 1);
  luv_registry_get_for_ptr(L, lhandle);
  lua_call(L, 1, 0);

#if 0
/* TODO: move this to where connection close. __gc? */
  luv_registry_delete_for_ptr(L, ((char *)handle) + 1);
  luv_registry_delete_for_ptr(L, handle);
#endif
printf("connection_cb exit\n");
}

static int luv_listen(lua_State *L) {
  int r;
  luv_stream_t *lhandle;
  int backlog;

printf("listen#1 L=%lx\n", (unsigned long)L);
  lhandle = lua_touserdata(L, 1);
printf("listen#2 handle=%lx\n", (unsigned long)&lhandle->handle);
  backlog = luaL_checkint(L, 2);

  luaL_checktype(L, 3, LUA_TFUNCTION);

  luv_registry_set_for_ptr(L, lhandle, 1);
  luv_registry_set_for_ptr(L, ((char *)lhandle) + 1, 3);

  r = uv_listen(&lhandle->handle, backlog, connection_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("listen#3 exit\n");
  return 0;
}

static int luv_accept(lua_State *L) {
  luv_stream_t *server;
  luv_stream_t *client;
  int r;

printf("accept#1 L=%lx\n", (unsigned long)L);
  server = lua_touserdata(L, 1);
  client = lua_touserdata(L, 2);
printf("accept#2 server=%lx, client=%lx\n", (unsigned long)&server->handle, (unsigned long)&client->handle);
  r = uv_accept(&server->handle, &client->handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("accept#3 exit\n");
  return 0;
}

static uv_buf_t alloc_cb(uv_handle_t *handle, size_t suggested_size) {
  lua_State *L;
  void *p;

printf("alloc_cb#1 for read, handle=%lx, suggested_size=%ld\n", (unsigned long)handle, suggested_size);
  L = handle->data;
printf("alloc_cb#2 L==%lx\n", (unsigned long)L);
  p = luv_buf_mem_alloc(L, suggested_size);
  if (!p)
    return uv_buf_init(NULL, 0);
  return uv_buf_init(p, suggested_size);
}

static void read_cb(uv_stream_t *handle, ssize_t nread, uv_buf_t buf) {
  luv_stream_t *lhandle;
  uv_loop_t *loop;
  lua_State *L;
  luv_stream_input_t *input;

printf("read_cb#1 handle=%lx, nread=%ld\n", (unsigned long)handle, nread);
  loop = handle->loop;
  L = handle->data;
printf("read_cb#2 L=%lx, loop=%lx, handle=%lx, nread=%ld\n", (unsigned long)L, (unsigned long)loop, (unsigned long)handle, nread);
  lhandle = container_of(handle, luv_stream_t, handle);

  input = luv_alloc(L, sizeof(luv_stream_input_t));
  if (!input)
    return;

  input->nread = nread;
  input->lbuf.orig = buf.base;
  input->lbuf.buf = buf;
  ngx_queue_insert_tail(&lhandle->input_queue, (ngx_queue_t *)input);

printf("read_cb#3 exiting or resume\n");
  if (lua_status(L) == LUA_YIELD && lhandle->is_yielded_for_read) {
printf("read_cb#4 resume yielded coroutine L=%lx\n", (unsigned long)L);
    lhandle->is_yielded_for_read = 0;
    luv_resume(L, L, 0);
  }
}

static int luv_read_start(lua_State *L) {
  luv_stream_t *lhandle;
  int r;

  lhandle = lua_touserdata(L, 1);
printf("read_start L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)&lhandle->handle);
  r = uv_read_start(&lhandle->handle, alloc_cb, read_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("read_start exit\n");
  return 0;
}

static int luv_read_stop(lua_State *L) {
  luv_stream_t *lhandle;
  int r;

  lhandle = lua_touserdata(L, 1);
printf("read_stop L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)&lhandle->handle);
  r = uv_read_stop(&lhandle->handle);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("read_stop exit\n");
  return 0;
}

static int luv_prim_read(lua_State *L) {
  luv_stream_t *lhandle;
  luv_stream_input_t *input;
  luv_buf_t *lbuf;

  lhandle = lua_touserdata(L, 1);
printf("prim_read#1 L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)&lhandle->handle);

  if (ngx_queue_empty(&lhandle->input_queue)) {
printf("prim_read#2 input_queue was empty, yield...\n");
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

printf("prim_read#3 return input\n");
  return 2;
}

static void write_cb(uv_write_t *req, int status) {
  lua_State *L;
  uv_stream_t *handle;

  handle = req->handle;
  L = handle->data;
printf("write_cb#1 L=%lx, handle=%lx, status=%d\n", (unsigned long)L, (unsigned long)handle, status);

  if (status < 0) {
    luv_free(L, req->data);
    luv_free(L, req);
printf("write_cb#2 error exit\n");
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
    return;
  }

  luv_free(L, req->data);
  luv_free(L, req);
printf("write_cb#3 exit\n");
  luv_resume(L, L, 0);
}

static int luv_write(lua_State *L) {
  uv_write_t *req;
  luv_stream_t *lhandle;
  uv_buf_t *bufs;
  size_t bufcnt;
  int r;

  lhandle = lua_touserdata(L, 1);
printf("write#1 L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)&lhandle->handle);
  bufs = luv_checkbuforstrtable(L, 2, &bufcnt);
luv_dbg_print_bufs("write#2", bufs, bufcnt);

  req = luv_alloc(L, sizeof(uv_write_t));
  req->data = bufs;

  r = uv_write(req, &lhandle->handle, bufs, (int)bufcnt, write_cb);
  if (r < 0) {
    luaL_error(L, luvL_uv_errname(uv_last_error(luv_loop(L)).code));
  }
printf("write exit\n");
  return lua_yield(L, 0);
}


static void close_cb(uv_handle_t *handle) {
  lua_State *L;

  L = handle->data;
printf("close_cb#1 resume L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)handle);
  luv_resume(L, L, 0);
}

static int luv_close(lua_State *L) {
  uv_handle_t *handle;

  handle = lua_touserdata(L, 1);
printf("close#1 L=%lx, handle=%lx\n", (unsigned long)L, (unsigned long)handle);
  uv_close(handle, close_cb);
printf("close#2 yield L=%lx\n", (unsigned long)L);
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
printf("luaopen_yaluv_native L=%lx\n", (unsigned long)L);
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
