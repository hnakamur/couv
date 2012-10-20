#include "couv-private.h"

#define COUV_FS_STAT_MTBL_NAME "couv.fs.Stat"
#define couv_checkfsstat(L, index) \
    (uv_statbuf_t *)luaL_checkudata(L, index, COUV_FS_STAT_MTBL_NAME)

typedef struct flag_mapping_s {
  char *name;
  int flags;
} flag_mapping_t;

static flag_mapping_t flag_mappings[] = {
   { "r",   O_RDONLY                      }
  ,{ "w",   O_WRONLY | O_CREAT | O_TRUNC  }
  ,{ "a",   O_WRONLY | O_CREAT | O_APPEND }
  ,{ "r+",  O_RDWR                        }
  ,{ "w+",  O_RDWR   | O_CREAT | O_TRUNC  }
  ,{ "a+",  O_RDWR   | O_CREAT | O_APPEND }
#ifndef _WIN32
  ,{ "rs",  O_RDONLY | O_SYNC             }
  ,{ "rs+", O_RDWR   | O_SYNC             }
#endif
  ,{ NULL,  0                             }
};

static int fs_toflags(const char *flags_str) {
  flag_mapping_t *mapping;
  for (mapping = flag_mappings; mapping->name; mapping++) {
    if (strcmp(flags_str, mapping->name) == 0) {
      return mapping->flags;
    }
  }
  return -1;
}

static int fs_checkflags(lua_State *L, int index) {
  const char *flags_str = luaL_checkstring(L, index);
  int flags = fs_toflags(flags_str);
  if (flags == -1) {
    return luaL_argerror(L, index, "Invalid flags");
  }
  return flags;
}

static int fs_checkmode(lua_State *L, int index) {
  int type = lua_type(L, index);
  int mode;
  if (type == LUA_TNUMBER) {
    mode = lua_tonumber(L, index);
  } else if (type == LUA_TSTRING) {
    size_t len;
    const char *str = lua_tolstring(L, index, &len);
    mode = strtoul(str, NULL, 8);
  } else {
    return luaL_argerror(L, index, "must be octal number string or number");
  }
  return mode;
}

static int fs_optmode(lua_State *L, int index, int default_value) {
  int type = lua_type(L, index);
  int mode;
  if (type == LUA_TNUMBER) {
    mode = lua_tonumber(L, index);
  } else if (type == LUA_TSTRING) {
    size_t len;
    const char *str = lua_tolstring(L, index, &len);
    mode = strtoul(str, NULL, 8);
  } else if (type == LUA_TNIL || type == LUA_TNONE) {
    mode = default_value;
  } else {
    return luaL_argerror(L, index, "must be octal number string or number");
  }
  return mode;
}

/*
 * fs stat methods
 */

static int fs_stat_dev(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_dev);
  return 1;
}

static int fs_stat_ino(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_ino);
  return 1;
}

static int fs_stat_mode(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_mode);
  return 1;
}

static int fs_stat_nlink(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_nlink);
  return 1;
}

static int fs_stat_gid(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_gid);
  return 1;
}

static int fs_stat_uid(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_uid);
  return 1;
}

static int fs_stat_rdev(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_rdev);
  return 1;
}

static int fs_stat_size(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_size);
  return 1;
}

#ifndef _WIN32
static int fs_stat_blksize(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_blksize);
  return 1;
}

static int fs_stat_blocks(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_blocks);
  return 1;
}
#endif

static int fs_stat_atime(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_atime);
  return 1;
}

static int fs_stat_mtime(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_mtime);
  return 1;
}

static int fs_stat_ctime(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_ctime);
  return 1;
}

static int fs_stat_is_file(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISREG(s->st_mode));
  return 1;
}

static int fs_stat_is_directory(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISDIR(s->st_mode));
  return 1;
}

static int fs_stat_is_character_device(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISCHR(s->st_mode));
  return 1;
}

static int fs_stat_is_block_device(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISBLK(s->st_mode));
  return 1;
}

static int fs_stat_is_fifo(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISFIFO(s->st_mode));
  return 1;
}

#ifndef _WIN32
static int fs_stat_is_symbolic_link(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISLNK(s->st_mode));
  return 1;
}

static int fs_stat_is_socket(lua_State *L) {
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISSOCK(s->st_mode));
  return 1;
}
#endif

static int fs_stat_permission (lua_State *L) {
  char permission_buf[16];
  uv_statbuf_t *s = couv_checkfsstat(L, 1);
  sprintf(permission_buf, "0%o", s->st_mode & ~S_IFMT);
  lua_pushstring(L, permission_buf);
  return 1;
}

static const struct luaL_Reg fs_stat_methods[] = {
  { "dev", fs_stat_dev },
  { "ino", fs_stat_ino },
  { "mode", fs_stat_mode },
  { "nlink", fs_stat_nlink },
  { "gid", fs_stat_gid },
  { "uid", fs_stat_uid },
  { "rdev", fs_stat_rdev },
  { "size", fs_stat_size },
#ifndef _WIN32
  { "blksize", fs_stat_blksize },
  { "blocks", fs_stat_blocks },
#endif
  { "atime", fs_stat_atime },
  { "mtime", fs_stat_mtime },
  { "ctime", fs_stat_ctime },
  { "isFile", fs_stat_is_file },
  { "isDirectory", fs_stat_is_directory },
  { "isCharacterDevice", fs_stat_is_character_device },
  { "isBlockDevice", fs_stat_is_block_device },
  { "isFIFO", fs_stat_is_fifo },
#ifndef _WIN32
  { "isSymbolicLink", fs_stat_is_symbolic_link },
  { "isSocket", fs_stat_is_socket },
#endif
  { "permission", fs_stat_permission },
  { NULL, NULL }
};


static int fs_push_readdir_results(lua_State *L, int entry_count,
    const char *entries) {
  const char *p;
  const char *q;
  int i;
  lua_createtable(L, entry_count, 0);
  for (i = 1, p = entries; i <= entry_count; ++i, p = q + 1) {
    q = strchr(p, '\0');
    if (!q) {
      return luaL_error(L, "invalid readdir results");
    }

    lua_pushlstring(L, p, q - p);
    lua_rawseti(L, -2, i);
  }
  return 1;
}

static int fs_push_exists_results(lua_State *L, uv_fs_t* req) {
  int exists;
  int is_async;
  couv_fs_t *w_req;

  exists = req->result == 0;

  is_async = req->cb != NULL;
  uv_fs_req_cleanup(req);
  if (is_async) {
    w_req = container_of(req, couv_fs_t, req);
    couv_free(L, w_req);
  }

  lua_pushboolean(L, exists);
  return 1;
}

static void fs_exists_callback(uv_fs_t* req) {
  lua_State *L;
  int nresults;

  L = req->data;
  nresults = fs_push_exists_results(L, req);
  couv_resume(L, L, nresults);
}

static int fs_error(lua_State *L, uv_fs_t* req) {
  int errcode;
  const char *errname;
  couv_fs_t *w_req;
  int nresults;
  int is_async;

  is_async = req->cb != NULL;
  errcode = is_async ? req->errorno : uv_last_error(req->loop).code;
  errname = couvL_uv_errname(errcode);
  switch (req->fs_type) {
  case UV_FS_OPEN:
  case UV_FS_READ:
  case UV_FS_WRITE:
  case UV_FS_LSTAT:
  case UV_FS_FSTAT:
  case UV_FS_STAT:
  case UV_FS_READLINK:
  case UV_FS_READDIR:
    lua_pushnil(L);
    lua_pushstring(L, errname);
    nresults = 2;
    break;
  default:
    lua_pushstring(L, errname);
    nresults = 1;
    break;
  }

  uv_fs_req_cleanup(req);
  if (is_async) {
    w_req = container_of(req, couv_fs_t, req);
    couv_free(L, w_req);
  }

  return nresults;
}

static int fs_push_common_results(lua_State *L, uv_fs_t* req) {
  int nresults;
  couv_fs_t *w_req;
  int is_async;

  if (req->result < 0)
    return fs_error(L, req);

  switch (req->fs_type) {
  case UV_FS_OPEN:
  case UV_FS_READ:
  case UV_FS_WRITE:
    lua_pushnumber(L, req->result);
    nresults = 1;
    break;
  case UV_FS_LSTAT:
  case UV_FS_FSTAT:
  case UV_FS_STAT: {
    uv_statbuf_t *stat_buf;
    stat_buf = (uv_statbuf_t *)lua_newuserdata(L, sizeof(uv_statbuf_t));
    *stat_buf = *(uv_statbuf_t *)req->ptr;
    luaL_getmetatable(L, COUV_FS_STAT_MTBL_NAME);
    lua_setmetatable(L, -2);
    nresults = 1;
    break;
  }
  case UV_FS_READLINK:
    lua_pushstring(L, req->ptr);
    nresults = 1;
    break;
  case UV_FS_READDIR:
    fs_push_readdir_results(L, req->result, req->ptr);
    nresults = 1;
    break;
  default:
    nresults = 0;
    break;
  }

  is_async = req->cb != NULL;
  uv_fs_req_cleanup(req);
  if (is_async) {
    w_req = container_of(req, couv_fs_t, req);
    couv_free(L, w_req);
  }

  return nresults;
}

static void fs_common_callback(uv_fs_t* req) {
  couv_fs_t *w_req;
  lua_State *L;
  int nresults;

  L = req->data;

  w_req = container_of(req, couv_fs_t, req);
  luaL_unref(L, LUA_REGISTRYINDEX, w_req->threadref);

  nresults = fs_push_common_results(L, req);
  couv_resume(L, L, nresults);
}

static uv_fs_t *fs_alloc_req(lua_State *L) {
  couv_fs_t *w_req;

  w_req = couv_alloc(L, sizeof(couv_fs_t));
  if (!w_req)
    return NULL;
  w_req->req.data = L;
  return &w_req->req;
}

typedef int (*fs_push_results_t)(lua_State *L, uv_fs_t* req);

static int fs_after_async_call(lua_State *L, uv_fs_t *req, int ret,
    fs_push_results_t push_func) {
  couv_fs_t *w_req;

  if (ret < 0) {
    lua_pop(L, 1); /* pop coroutine. */
    return push_func(L, req);
  }

  w_req = container_of(req, couv_fs_t, req);
  w_req->threadref = luaL_ref(L, LUA_REGISTRYINDEX);
  return lua_yield(L, 0);
}

static int fs_chown(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int uid = luaL_checkint(L, 2);
  int gid = luaL_checkint(L, 3);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_chown(loop, &req, path, uid, gid, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_chown(loop, req, path, uid, gid, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_chmod(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int mode = fs_checkmode(L, 2);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_chmod(loop, &req, path, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_chmod(loop, req, path, mode, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_close(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_close(loop, &req, fd, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_close(loop, req, fd, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_exists(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_stat(loop, &req, path, NULL);
    return fs_push_exists_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_stat(loop, req, path, fs_exists_callback);
    return fs_after_async_call(L, req, r, fs_push_exists_results);
  }
}

static int fs_fchmod(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  int mode = fs_checkmode(L, 2);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fchmod(loop, &req, fd, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fchmod(loop, req, fd, mode, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_fchown(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  int uid = luaL_checkint(L, 2);
  int gid = luaL_checkint(L, 3);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fchown(loop, &req, fd, uid, gid, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fchown(loop, req, fd, uid, gid, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_fstat(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fstat(loop, &req, fd, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fstat(loop, req, fd, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_fsync(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fsync(loop, &req, fd, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fsync(loop, req, fd, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_ftruncate(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  long len = luaL_optlong(L, 2, 0);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_ftruncate(loop, &req, fd, len, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_ftruncate(loop, req, fd, len, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_futime(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  double atime = luaL_checknumber(L, 2);
  double mtime = luaL_checknumber(L, 3);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_futime(loop, &req, fd, atime, mtime, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_futime(loop, req, fd, atime, mtime, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_link(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  const char *new_path = luaL_checkstring(L, 2);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_link(loop, &req, path, new_path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_link(loop, req, path, new_path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_lstat(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_lstat(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_lstat(loop, req, path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_mkdir(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int mode = fs_optmode(L, 2, 0777);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_mkdir(loop, &req, path, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_mkdir(loop, req, path, mode, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_open(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int flags = fs_checkflags(L, 2);
  int mode = fs_optmode(L, 3, 0666);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_open(loop, &req, path, flags, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_open(loop, req, path, flags, mode, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_readlink(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_readlink(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_readlink(loop, req, path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_rename(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *old_path = luaL_checkstring(L, 1);
  const char *new_path = luaL_checkstring(L, 2);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_rename(loop, &req, old_path, new_path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_rename(loop, req, old_path, new_path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_rmdir(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_rmdir(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_rmdir(loop, req, path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_stat(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_stat(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_stat(loop, req, path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_symlink(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  const char *new_path = luaL_checkstring(L, 2);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_symlink(loop, &req, path, new_path, 0, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_symlink(loop, req, path, new_path, 0, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_unlink(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_unlink(loop, req, path, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_utime(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  double atime = luaL_checknumber(L, 2);
  double mtime = luaL_checknumber(L, 3);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_utime(loop, &req, path, atime, mtime, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_utime(loop, req, path, atime, mtime, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_read(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  uv_buf_t *buf = couv_checkbuf(L, 2);
  char *p = buf->base;
  int buf_pos = luaL_optint(L, 3, 1);
  int length = luaL_optint(L, 4, buf->len - buf_pos + 1);
  long file_offset = luaL_optlong(L, 5, -1);
  couv_argcheckindex(L, 3, buf_pos, 1, buf->len);
  couv_argcheckindex(L, 4, length, 0, buf->len - buf_pos + 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_read(loop, &req, fd, (void *)&p[buf_pos - 1], length, file_offset,
        NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_read(loop, req, fd, (void *)&p[buf_pos - 1], length,
        file_offset, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_readdir(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_readdir(loop, &req, path, 0, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_readdir(loop, req, path, 0, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static int fs_write(lua_State *L) {
  uv_loop_t *loop = couv_loop(L);
  int fd = luaL_checkint(L, 1);
  uv_buf_t buf = couv_checkbuforstr(L, 2);
  int buf_pos = luaL_optint(L, 3, 1);
  int buflen = (int)buf.len;
  char *p = buf.base;
  int length = luaL_optint(L, 4, buflen - buf_pos + 1);
  long file_offset = luaL_optlong(L, 5, -1);
  couv_argcheckindex(L, 3, buf_pos, 1, (int)buflen);
  couv_argcheckindex(L, 4, length, 0, (int)buflen - buf_pos + 1);
  if (couvL_is_mainthread(L)) {
    uv_fs_t req;
    uv_fs_write(loop, &req, fd, (void *)&p[buf_pos - 1], length, file_offset,
        NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_write(loop, req, fd, (void *)&p[buf_pos - 1], length,
        file_offset, fs_common_callback);
    return fs_after_async_call(L, req, r, fs_error);
  }
}

static const struct luaL_Reg fs_functions[] = {
  { "fs_chmod", fs_chmod },
  { "fs_chown", fs_chown },
  { "fs_close", fs_close },
  { "fs_exists", fs_exists },
  { "fs_fchmod", fs_fchmod },
  { "fs_fchown", fs_fchown },
  { "fs_fstat", fs_fstat },
  { "fs_fsync", fs_fsync },
  { "fs_ftruncate", fs_ftruncate },
  { "fs_futime", fs_futime },
  { "fs_link", fs_link },
  { "fs_lstat", fs_lstat },
  { "fs_mkdir", fs_mkdir },
  { "fs_open", fs_open },
  { "fs_read", fs_read },
  { "fs_readdir", fs_readdir },
  { "fs_readlink", fs_readlink },
  { "fs_rename", fs_rename },
  { "fs_rmdir", fs_rmdir },
  { "fs_stat", fs_stat },
  { "fs_symlink", fs_symlink },
  { "fs_unlink", fs_unlink },
  { "fs_utime", fs_utime },
  { "fs_write", fs_write },
  { NULL, NULL }
};

int luaopen_couv_fs(lua_State *L) {
  couvL_setfuncs(L, fs_functions, 0);

  luaL_newmetatable(L, COUV_FS_STAT_MTBL_NAME);
  couvL_setfuncs(L, fs_stat_methods, 0);
  lua_setfield(L, -1, "__index");
  return 1;
}
