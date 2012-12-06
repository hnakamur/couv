#include "couv-private.h"

ngx_queue_t *get_io_result_queue(lua_State *L) {
  ngx_queue_t *queue;

  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_IO_RESULT_QUEUE_REG_KEY(L));
  queue = lua_touserdata(L, -1);
  lua_pop(L, 1);
  if (!queue) {
    queue = lua_newuserdata(L, sizeof(ngx_queue_t));
    ngx_queue_init(queue);
    couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_IO_RESULT_QUEUE_REG_KEY(L));
  }
  return queue;
}

static void couv_free_write_res(lua_State *L, couv_write_result_t *res) {
  uv_write_t *req;

  req = res->req;
  couv_free(L, req->data);
  couv_free(L, req);
}

static couv_io_result_t *take_io_result(lua_State *L, couv_io_op_code op_code) {
  ngx_queue_t *queue;
  couv_io_result_t *res;

  queue = get_io_result_queue(L);
  for (res = (couv_io_result_t *)ngx_queue_head(queue);
       res != (couv_io_result_t *)ngx_queue_sentinel(queue);
       res = (couv_io_result_t *)ngx_queue_next(res)) {
    if (res->op_code == op_code) {
      ngx_queue_remove(res);
      return res;
    }
  }
  return NULL;
}

int couv_take_write_res(lua_State *L) {
  couv_write_result_t *res;

  res = take_io_result(L, COUV_WRITE_OP);
  if (!res)
    return lua_yield(L, 0);
  couv_free_write_res(L, res);
  return 0;
}

