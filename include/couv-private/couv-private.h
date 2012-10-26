#ifndef _COUV_PRIVATE_H
#define _COUV_PRIVATE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "couv-win.h"
#else
#include "couv-unix.h"
#endif

#include <lauxlib.h>
#include "uv.h"

#include "couv.h"
#include "ngx-queue.h"

/*
 * auxlib
 */
#if LUA_VERSION_NUM == 502

#define couv_rawlen lua_rawlen
#define couvL_setfuncs(L, l, nup) luaL_setfuncs(L, l, nup)
#define couvL_testudata luaL_testudata
#define couv_absindex lua_absindex
#define couv_resume(L, from, nargs) lua_resume(L, from, nargs)
#define couv_rawsetp lua_rawsetp
#define couv_rawgetp lua_rawgetp

#elif LUA_VERSION_NUM == 501

#define couv_rawlen lua_objlen
void couvL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
void *couvL_testudata (lua_State *L, int ud, const char *tname);
int couv_absindex(lua_State *L, int idx);
#define couv_resume(L, from, nargs) lua_resume(L, nargs)
void couv_rawsetp(lua_State *L, int index, const void *p);
void couv_rawgetp(lua_State *L, int index, const void *p);

#else

#error

#endif

void *couv_alloc(lua_State *L, size_t size);
void couv_free(lua_State *L, void *ptr);

/* return 1 if mainthread, 0 otherwise and push coroutine. */
int couvL_is_mainthread(lua_State *L);

int couvL_hasmetatablename(lua_State *L, int index, const char *tname);
const char *couvL_uv_errname(int uv_errcode);

int couv_newmetatable(lua_State *L, const char *tname, const char *super_tname);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#define couvL_SET_FIELD(L, name, type, val) \
  lua_push##type(L, val);                   \
  lua_setfield(L, -2, #name)

typedef void *(*couv_alloc_t)(lua_State *L, size_t nbytes);
typedef void (*couv_free_t)(lua_State *L, void *ptr);

typedef struct couv_buf_mem_s couv_buf_mem_t;
struct couv_buf_mem_s {
  int ref_cnt;
  char mem[1];
};

void *couv_buf_mem_alloc(lua_State *L, size_t nbytes);
void couv_buf_mem_retain(lua_State *L, void *ptr);
void couv_buf_mem_release(lua_State *L, void *ptr);

uv_buf_t couv_buf_alloc_cb(uv_handle_t* handle, size_t suggested_size);

typedef struct couv_buf_s {
  void *orig;
  uv_buf_t buf;
} couv_buf_t;

typedef struct couv_udp_input_s {
  ngx_queue_t *prev;
  ngx_queue_t *next;
  ssize_t nread;
  couv_buf_t w_buf;
  union {
    struct sockaddr_storage storage;
    struct sockaddr_in v4;
    struct sockaddr_in6 v6;
  } addr;
} couv_udp_input_t;

typedef struct couv_stream_input_s {
  ngx_queue_t *prev;
  ngx_queue_t *next;
  ssize_t nread;
  couv_buf_t w_buf;
} couv_stream_input_t;

typedef struct couv_pipe_input_s {
  ngx_queue_t *prev;
  ngx_queue_t *next;
  ssize_t nread;
  couv_buf_t w_buf;
  uv_handle_type pending;
} couv_pipe_input_t;

/*
 * handle data.
 */
#define COUV_UDP_HANDLE_DATA_FIELDS \
  ngx_queue_t input_queue;          \
  int is_yielded_for_input;

#define COUV_STREAM_HANDLE_DATA_FIELDS \
  ngx_queue_t input_queue;             \
  int is_yielded_for_input;

typedef struct couv_udp_handle_data_s {
  COUV_UDP_HANDLE_DATA_FIELDS 
} couv_udp_handle_data_t;

typedef struct couv_stream_handle_data_s {
  COUV_STREAM_HANDLE_DATA_FIELDS 
} couv_stream_handle_data_t;

/*
 * handles.
 */
#define COUV_HANDLE_METATABLE_NAME "couv.Handle"
#define COUV_STREAM_METATABLE_NAME "couv.Stream"
#define COUV_TCP_METATABLE_NAME "couv.Tcp"
#define COUV_UDP_METATABLE_NAME "couv.Udp"
#define COUV_TTY_METATABLE_NAME "couv.Tty"
#define COUV_PIPE_METATABLE_NAME "couv.Pipe"
#define COUV_PROCESS_METATABLE_NAME "couv.Process"
#define COUV_TIMER_METATABLE_NAME "couv.Timer"

typedef struct couv_handle_s {
  uv_handle_t handle;
} couv_handle_t;

typedef struct couv_udp_s {
  uv_udp_t handle;
  couv_udp_handle_data_t hdata;
} couv_udp_t;

typedef struct couv_pipe_s {
  uv_pipe_t handle;
  couv_stream_handle_data_t hdata;
} couv_pipe_t;

typedef struct couv_tcp_s {
  uv_tcp_t handle;
  couv_stream_handle_data_t hdata;
} couv_tcp_t;

typedef struct couv_tty_s {
  uv_tty_t handle;
  couv_stream_handle_data_t hdata;
} couv_tty_t;

couv_stream_handle_data_t *couv_get_stream_handle_data(uv_stream_t *handle);

/*
 * handle registry keys.
 */
#define COUV_USERDATA_REG_KEY(h)  h
#define COUV_THREAD_REG_KEY(h)    (((char *)h) + 1)
#define COUV_LISTEN_CB_REG_KEY(h) (((char *)h) + 2)
#define COUV_TIMER_CB_REG_KEY(h)  (((char *)h) + 2)
#define COUV_EXIT_CB_REG_KEY(h)   (((char *)h) + 2)

void couv_clean_process_handle(lua_State *L, uv_process_t *handle);
void couv_clean_tcp_handle(lua_State *L, uv_tcp_t *handle);
void couv_clean_timer_handle(lua_State *L, uv_timer_t *handle);
void couv_clean_tty_handle(lua_State *L, uv_tty_t *handle);
void couv_clean_udp_handle(lua_State *L, uv_udp_t *handle);
void couv_clean_pipe_handle(lua_State *L, uv_pipe_t *handle);

/*
 * buffer
 */
int luaopen_couv_buffer(lua_State *L);

#define COUV_BUFFER_MTBL_NAME "couv.Buffer"
#define couv_checkbuf(L, index) \
    luaL_checkudata(L, index, COUV_BUFFER_MTBL_NAME)
uv_buf_t couv_tobuforstr(lua_State *L, int index);
uv_buf_t couv_checkbuforstr(lua_State *L, int index);

/* NOTE: you must free the result buffers array with couv_free. */
uv_buf_t *couv_checkbuforstrtable(lua_State *L, int index, size_t *buffers_cnt);

void couv_dbg_print_bufs(const char *header, uv_buf_t *bufs, size_t bufcnt);

#define couv_argcheckindex(L, arg_index, index, min, max) \
  luaL_argcheck(L, (int)min <= index && index <= (int)max, arg_index, \
      "index out of range");


/*
 * loop
 */
int luaopen_couv_loop(lua_State *L);
uv_loop_t *couv_loop(lua_State *L);

#define COUV_LOOP_REGISTRY_KEY "couv.loop"

/*
 * ipaddr
 */
#define COUV_IP4ADDR_MTBL_NAME "couv.Ip4addr"
#define couv_checkip4addr(L, index) \
  (struct sockaddr_in *)luaL_checkudata(L, index, COUV_IP4ADDR_MTBL_NAME)

#define COUV_IP6ADDR_MTBL_NAME "couv.Ip6addr"
#define couv_checkip6addr(L, index) \
  (struct sockaddr_in6 *)luaL_checkudata(L, index, COUV_IP6ADDR_MTBL_NAME)

#define couvL_testip4addr(L, index) \
  (struct sockaddr_in *)couvL_testudata(L, index, COUV_IP4ADDR_MTBL_NAME)
#define couvL_testip6addr(L, index) \
  (struct sockaddr_in6 *)couvL_testudata(L, index, COUV_IP6ADDR_MTBL_NAME)

int luaopen_couv_ipaddr(lua_State *L);

int couv_dbg_print_ip4addr(const char *header, struct sockaddr_in *addr);

/*
 * fs
 */
int luaopen_couv_fs(lua_State *L);

typedef struct couv_fs_s {
  int threadref;
  uv_fs_t req;
} couv_fs_t;


int luaopen_couv_handle(lua_State *L);
int luaopen_couv_pipe(lua_State *L);
int luaopen_couv_process(lua_State *L);
int luaopen_couv_stream(lua_State *L);
int luaopen_couv_tcp(lua_State *L);
int luaopen_couv_timer(lua_State *L);
int luaopen_couv_tty(lua_State *L);
int luaopen_couv_udp(lua_State *L);

int couv_push_ipaddr_raw(lua_State *L, struct sockaddr *addr);

#ifdef __cplusplus
}
#endif
#endif /* _COUV_PRIVATE_H */
