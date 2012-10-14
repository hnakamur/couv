#ifndef _YALUV_PRIVATE_H
#define _YALUV_PRIVATE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <lauxlib.h>
#include <uv.h>

#include "yaluv.h"
#include "ngx-queue.h"

typedef void *(*luv_alloc_t)(lua_State *L, size_t nbytes);
typedef void (*luv_free_t)(lua_State *L, void *ptr);

typedef struct luv_buf_mem_s luv_buf_mem_t;
struct luv_buf_mem_s {
  int ref_cnt;
  luv_free_t free;
  char mem[1];
};

void *luv_buf_mem_alloc(lua_State *L, size_t nbytes);
void luv_buf_mem_retain(lua_State *L, void *ptr);
void luv_buf_mem_release(lua_State *L, void *ptr);

typedef struct luv_buf_s {
  void *orig;
  uv_buf_t buf;
} luv_buf_t;

typedef struct luv_udp_input_s {
  ngx_queue_t *prev;
  ngx_queue_t *next;
  ssize_t nread;
  luv_buf_t lbuf;
  union {
    struct sockaddr_storage storage;
    struct sockaddr_in v4;
    struct sockaddr_in6 v6;
  } addr;
} luv_udp_input_t;

typedef struct luv_udp_s {
  uv_udp_t handle;
  int is_yielded_for_recv;
  ngx_queue_t input_queue;
} luv_udp_t;

typedef struct luv_stream_input_s {
  ngx_queue_t *prev;
  ngx_queue_t *next;
  ssize_t nread;
  luv_buf_t lbuf;
} luv_stream_input_t;

typedef struct luv_stream_s {
  uv_stream_t handle;
  int is_yielded_for_read;
  ngx_queue_t input_queue;
} luv_stream_t;

typedef struct luv_tcp_s {
  uv_tcp_t handle;
  int is_yielded_for_read;
  ngx_queue_t input_queue;
} luv_tcp_t;


/*
 * buffer
 */
int luaopen_yaluv_buffer(lua_State *L);

#define LUV_BUFFER_MTBL_NAME "luv.Buffer"
#define luv_checkbuf(L, index) \
    luaL_checkudata(L, index, LUV_BUFFER_MTBL_NAME)
uv_buf_t luv_tobuforstr(lua_State *L, int index);
uv_buf_t luv_checkbuforstr(lua_State *L, int index);

/* NOTE: you must free the result buffers array with luv_free. */
uv_buf_t *luv_checkbuforstrtable(lua_State *L, int index, size_t *buffers_cnt);

void luv_dbg_print_bufs(const char *header, uv_buf_t *bufs, size_t bufcnt);

#define luv_argcheckindex(L, arg_index, index, min, max) \
  luaL_argcheck(L, (int)min <= index && index <= (int)max, arg_index, \
      "index out of range");


/*
 * loop
 */
int luaopen_yaluv_loop(lua_State *L);
uv_loop_t *luv_loop(lua_State *L);

#define LUV_LOOP_MTBL_NAME "luv.loop.Loop"
#define luv_checkloop(L, index) \
    (*(uv_loop_t **)luaL_checkudata(L, index, LUV_LOOP_MTBL_NAME))

#if LUA_VERSION_NUM == 502
#define luv_resume(L, from, nargs) lua_resume(L, from, nargs)
#elif LUA_VERSION_NUM == 501
#define luv_resume(L, from, nargs) lua_resume(L, nargs)
#else
#error
#endif

/*
 * ipaddr
 */
#define LUV_IP4ADDR_MTBL_NAME "luv.Ip4addr"
#define luv_checkip4addr(L, index) \
  (struct sockaddr_in *)luaL_checkudata(L, index, LUV_IP4ADDR_MTBL_NAME)

#define LUV_IP6ADDR_MTBL_NAME "luv.Ip6addr"
#define luv_checkip6addr(L, index) \
  (struct sockaddr_in6 *)luaL_checkudata(L, index, LUV_IP6ADDR_MTBL_NAME)

int luaopen_yaluv_ipaddr(lua_State *L);

int ip4addr_dbg_print(const char *header, struct sockaddr_in *addr);

/*
 * fs
 */
int luaopen_yaluv_fs(lua_State *L);

/*
 * tcp
 */
int luaopen_yaluv_tcp(lua_State *L);

/*
 * udp
 */
int luaopen_yaluv_udp(lua_State *L);

/*
 * auxlib
 */
void *luv_alloc(lua_State *L, size_t size);
void luv_free(lua_State *L, void *ptr);

int luvL_is_in_mainthread(lua_State *L);
int luvL_hasmetatablename(lua_State *L, int index, const char *tname);
const char *luvL_uv_errname(int uv_errcode);

void luv_registry_set_for_ptr(lua_State *L, void *ptr, int index);
void luv_registry_get_for_ptr(lua_State *L, void *ptr);
void luv_registry_delete_for_ptr(lua_State *L, void *ptr);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#define luvL_SET_FIELD(L, name, type, val) \
  lua_push##type(L, val);                  \
  lua_setfield(L, -2, #name)

#ifdef __cplusplus
}
#endif
#endif /* _YALUV_PRIVATE_H */
