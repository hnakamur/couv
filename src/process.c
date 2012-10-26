#include "couv-private.h"

static uv_process_t *couv_new_process_handle(lua_State *L) {
  uv_process_t *handle;

  handle = lua_newuserdata(L, sizeof(uv_process_t));
  if (!handle)
    return NULL;

  lua_getfield(L, LUA_REGISTRYINDEX, COUV_PROCESS_MTBL_NAME);
  lua_setmetatable(L, -2);

  return handle;
}

void couv_clean_process_handle(lua_State *L, uv_process_t *handle) {
  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));

  lua_pushnil(L);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_EXIT_CB_REG_KEY(handle));
}

static const char **couv_tonullterminatedstrarray(lua_State *L, int index,
    int *cnt) {
  int type;
  int i;
  int n;
  const char *arg;
  const char **args;

  type = lua_type(L, index);
  if (type == LUA_TNIL || type == LUA_TNONE) {
    if (cnt)
      *cnt = 0;
    return NULL;
  } else if (type != LUA_TTABLE) {
    luaL_argerror(L, index, "must be nil or array of strings");
    return NULL;
  }

  n = couv_rawlen(L, index);
  args = couv_alloc(L, (n + 1) * sizeof(char *));
  if (!args)
    return NULL;

  for (i = 1; i <= n; ++i) {
    lua_rawgeti(L, index, i);
    arg = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (!arg) {
      couv_free(L, args);
      luaL_argerror(L, index, "must be nil or array of strings");
      return NULL;
    }
      
    args[i - 1] = arg;
  }
  args[n] = NULL;
  if (cnt)
    *cnt = n;
  return args;
}

static void exit_cb(uv_process_t *handle, int exit_status, int term_signal) {
  lua_State *L;

  L = handle->data;
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_EXIT_CB_REG_KEY(handle));
  couv_rawgetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));
  lua_pushnumber(L, exit_status);
  lua_pushnumber(L, term_signal);
  lua_call(L, 3, 0);
}

static uv_stdio_container_t *couv_checkstdiocontainers(lua_State *L, int index,
    int *cnt) {
  int stdio_cnt;
  uv_stdio_container_t *stdio;
  int i;
  int elem_cnt;
  int type;

  lua_getfield(L, index, "stdio");
  if (lua_isnil(L, -1))
    return NULL;
  luaL_argcheck(L, lua_istable(L, -1), index,
      "value at \"stdio\" key must be nil or table");

  stdio_cnt = couv_rawlen(L, -1);
  stdio = couv_alloc(L, stdio_cnt * sizeof(uv_stdio_container_t));
  if (!stdio)
    return NULL;

  memset(stdio, 0, stdio_cnt * sizeof(uv_stdio_container_t));

#define STDIO_INVAL_ELEM "invalid array element value at \"stdio\" key"
  for (i = 1; i <= stdio_cnt; ++i) {
    lua_rawgeti(L, -1, i);
    luaL_argcheck(L, lua_istable(L, -1), index, STDIO_INVAL_ELEM);
    elem_cnt = couv_rawlen(L, -1);
    luaL_argcheck(L, elem_cnt == 1 || elem_cnt == 2, index, STDIO_INVAL_ELEM);

    lua_rawgeti(L, -1, 1);
    luaL_argcheck(L, lua_isnumber(L, -1), index, STDIO_INVAL_ELEM);
    stdio[i - 1].flags = lua_tointeger(L, -1);
    lua_pop(L, 1);

    if (elem_cnt == 2) {
      lua_rawgeti(L, -1, 2);
      type = lua_type(L, -1);
      if (type == LUA_TNUMBER)
        stdio[i - 1].data.fd = lua_tointeger(L, -1);
      else if (type == LUA_TUSERDATA) {
        stdio[i - 1].data.stream =
            couvL_checkudataclass(L, -1, COUV_STREAM_MTBL_NAME);
      }
      lua_pop(L, 1);
    }

    lua_pop(L, 1);
  }
  if (cnt)
    *cnt = stdio_cnt;
  return stdio;
}

static int couv_spawn(lua_State *L) {
  uv_loop_t *loop;
  uv_process_t *handle;
  uv_process_options_t options;
  int argc;
  int r;

  handle = couv_new_process_handle(L);
  if (!handle)
    return 0;

  if (lua_type(L, 1) != LUA_TTABLE)
    luaL_argerror(L, 1, "must be table");

  memset(&options, 0, sizeof(uv_process_options_t));

  lua_getfield(L, 1, "args");
  options.args = (char **)couv_tonullterminatedstrarray(L, -1, &argc);
  lua_pop(L, 1);
  luaL_argcheck(L, argc, -1,
      "must have non-empty string array at key \"args\"");

  lua_getfield(L, 1, "file");
  options.file = lua_tostring(L, -1);
  lua_pop(L, 1);
  if (!options.file)
    options.file = options.args[0];

  lua_getfield(L, 1, "env");
  options.env = (char **)couv_tonullterminatedstrarray(L, -1, NULL);
  lua_pop(L, 1);

  lua_getfield(L, 1, "cwd");
  options.cwd = (char *)lua_tostring(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "flags");
  options.flags = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "uid");
  options.uid = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "gid");
  options.gid = lua_tointeger(L, -1);
  lua_pop(L, 1);

  options.stdio = couv_checkstdiocontainers(L, 1, &options.stdio_count);
  lua_pop(L, 1);

  lua_getfield(L, 1, "exitCb");
  if (lua_isfunction(L, -1)) {
    options.exit_cb = exit_cb;
    couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_EXIT_CB_REG_KEY(handle));
  } else
    luaL_argerror(L, 1, "value at \"exitCb\" key must be function");

  loop = couv_loop(L);
  r = uv_spawn(loop, handle, options);
  if (r < 0)
    return luaL_error(L, couvL_uv_errname(uv_last_error(loop).code));
  if (options.args)
    couv_free(L, options.args);
  if (options.env)
    couv_free(L, options.env);
  if (options.stdio)
    couv_free(L, options.stdio);
  handle->data = L;

  lua_pushvalue(L, -1);
  couv_rawsetp(L, LUA_REGISTRYINDEX, COUV_USERDATA_REG_KEY(handle));

  return 1;
}

static int couv_process_kill(lua_State *L) {
  uv_process_t *handle;
  int signum;
  int r;

  handle = couvL_checkudataclass(L, 1, COUV_PROCESS_MTBL_NAME);
  signum = luaL_checkint(L, 2);
  r = uv_process_kill(handle, signum);
  if (r < 0)
    return luaL_error(L, couvL_uv_errname(uv_last_error(couv_loop(L)).code));

  return 0;
}

static int couv_get_pid(lua_State *L) {
  uv_process_t *handle;

  handle = couvL_checkudataclass(L, 1, COUV_PROCESS_MTBL_NAME);
  lua_pushnumber(L, handle->pid);
  return 1;
}

static const struct luaL_Reg process_functions[] = {
  { "spawn", couv_spawn },
  { "get_pid", couv_get_pid },
  { "process_kill", couv_process_kill },
  { NULL, NULL }
};

static int set_process_flags_contants(lua_State *L) {
  couvL_SET_FIELD(L, PROCESS_SETUID, number, UV_PROCESS_SETUID);
  couvL_SET_FIELD(L, PROCESS_SETGID, number, UV_PROCESS_SETGID);
  couvL_SET_FIELD(L, PROCESS_WINDOWS_VERBATIM_ARGUMENTS, number,
      UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS);
  couvL_SET_FIELD(L, PROCESS_DETACHED, number, UV_PROCESS_DETACHED);
  return 0;
}

static int set_stdio_flags_contants(lua_State *L) {
  couvL_SET_FIELD(L, IGNORE, number, UV_IGNORE);
  couvL_SET_FIELD(L, CREATE_PIPE, number, UV_CREATE_PIPE);
  couvL_SET_FIELD(L, INHERIT_FD, number, UV_INHERIT_FD);
  couvL_SET_FIELD(L, INHERIT_STREAM, number, UV_INHERIT_STREAM);
  couvL_SET_FIELD(L, READABLE_PIPE, number, UV_READABLE_PIPE);
  couvL_SET_FIELD(L, WRITABLE_PIPE, number, UV_WRITABLE_PIPE);
  return 0;
}

int luaopen_couv_process(lua_State *L) {
  couv_newmetatable(L, COUV_PROCESS_MTBL_NAME, COUV_HANDLE_METATABLE_NAME);
  lua_pop(L, 1);

  set_process_flags_contants(L);
  set_stdio_flags_contants(L);

  couvL_setfuncs(L, process_functions, 0);
  return 1;
}
