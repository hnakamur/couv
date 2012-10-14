#include "yaluv-private.h"

#define LUV_FS_STAT_MTBL_NAME "luv.fs.Stat"
#define luv_checkfsstat(L, index) \
    (uv_statbuf_t *)luaL_checkudata(L, index, LUV_FS_STAT_MTBL_NAME)

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
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_dev);
  return 1;
}

static int fs_stat_ino(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_ino);
  return 1;
}

static int fs_stat_mode(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_mode);
  return 1;
}

static int fs_stat_nlink(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_nlink);
  return 1;
}

static int fs_stat_gid(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_gid);
  return 1;
}

static int fs_stat_uid(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_uid);
  return 1;
}

static int fs_stat_rdev(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_rdev);
  return 1;
}

static int fs_stat_size(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_size);
  return 1;
}

static int fs_stat_blksize(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_blksize);
  return 1;
}

static int fs_stat_blocks(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_blocks);
  return 1;
}

static int fs_stat_atime(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_atime);
  return 1;
}

static int fs_stat_mtime(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_mtime);
  return 1;
}

static int fs_stat_ctime(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushnumber(L, s->st_ctime);
  return 1;
}

static int fs_stat_is_file(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISREG(s->st_mode));
  return 1;
}

static int fs_stat_is_directory(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISDIR(s->st_mode));
  return 1;
}

static int fs_stat_is_character_device(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISCHR(s->st_mode));
  return 1;
}

static int fs_stat_is_block_device(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISBLK(s->st_mode));
  return 1;
}

static int fs_stat_is_fifo(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISFIFO(s->st_mode));
  return 1;
}

static int fs_stat_is_symbolic_link(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISLNK(s->st_mode));
  return 1;
}

static int fs_stat_is_socket(lua_State *L) {
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
  lua_pushboolean(L, S_ISSOCK(s->st_mode));
  return 1;
}

static int fs_stat_permission (lua_State *L) {
  char permission_buf[16];
  uv_statbuf_t *s = luv_checkfsstat(L, 1);
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
  { "blksize", fs_stat_blksize },
  { "blocks", fs_stat_blocks },
  { "atime", fs_stat_atime },
  { "mtime", fs_stat_mtime },
  { "ctime", fs_stat_ctime },
  { "isFile", fs_stat_is_file },
  { "isDirectory", fs_stat_is_directory },
  { "isCharacterDevice", fs_stat_is_character_device },
  { "isBlockDevice", fs_stat_is_block_device },
  { "isFIFO", fs_stat_is_fifo },
  { "isSymbolicLink", fs_stat_is_symbolic_link },
  { "isSocket", fs_stat_is_socket },
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
  lua_pushboolean(L, req->result == 0);
  return 1;
}

static void fs_exists_callback(uv_fs_t* req) {
  lua_State *L = (lua_State *)req->data;
  int nresults = fs_push_exists_results(L, req);
  luv_free(L, req);
  luv_resume(L, L, nresults);
}

static int fs_push_common_results(lua_State *L, uv_fs_t* req) {
  int nresults;
  if (req->result < 0) {
    int is_async = req->cb != NULL;
    int errcode = is_async ? req->errorno : uv_last_error(req->loop).code;
    lua_pushstring(L, luvL_uv_errname(errcode));
    nresults = 1;
  } else {
    switch (req->fs_type) {
    case UV_FS_OPEN:
    case UV_FS_READ:
    case UV_FS_WRITE:
      lua_pushnil(L);
      lua_pushnumber(L, req->result);
      nresults = 2;
      break;
    case UV_FS_LSTAT:
    case UV_FS_FSTAT:
    case UV_FS_STAT: {
      uv_statbuf_t *stat_buf;
      lua_pushnil(L);
      stat_buf = (uv_statbuf_t *)lua_newuserdata(L, sizeof(uv_statbuf_t));
      *stat_buf = *(uv_statbuf_t *)req->ptr;
      luaL_getmetatable(L, LUV_FS_STAT_MTBL_NAME);
      lua_setmetatable(L, -2);
      nresults = 2;
      break;
    }
    case UV_FS_READLINK:
      lua_pushnil(L);
      lua_pushstring(L, req->ptr);
      nresults = 2;
      break;
    case UV_FS_READDIR:
      lua_pushnil(L);
      fs_push_readdir_results(L, req->result, req->ptr);
      nresults = 2;
      break;
    default:
      nresults = 0;
      break;
    }
  }
  uv_fs_req_cleanup(req);
  return nresults;
}

static void fs_common_callback(uv_fs_t* req) {
  lua_State *L = (lua_State *)req->data;
  int nresults = fs_push_common_results(L, req);
  luv_free(L, req);
  luv_resume(L, L, nresults);
}

static int fs_yield_or_error(uv_fs_t *req, int ret_code) {
  lua_State *L = (lua_State *)req->data;
  if (ret_code < 0) {
    lua_pushstring(L, luvL_uv_errname(uv_last_error(req->loop).code));
    luv_free(L, req);
    return 1;
  }
  return lua_yield(L, 0);
}

static uv_fs_t *fs_alloc_req(lua_State *L) {
  uv_fs_t *req = luv_alloc(L, sizeof(uv_fs_t));
  if (!req)
    return NULL;
  req->data = L;
  return req;
}

static int fs_chown(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int uid = luaL_checkint(L, 2);
  int gid = luaL_checkint(L, 3);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_chown(loop, &req, path, uid, gid, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_chown(loop, req, path, uid, gid, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_chmod(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int mode = fs_checkmode(L, 2);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_chmod(loop, &req, path, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_chmod(loop, req, path, mode, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_close(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_close(loop, &req, fd, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_close(loop, req, fd, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_exists(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_stat(loop, &req, path, NULL);
    return fs_push_exists_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_stat(loop, req, path, fs_exists_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_fchmod(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  int mode = fs_checkmode(L, 2);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fchmod(loop, &req, fd, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fchmod(loop, req, fd, mode, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_fchown(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  int uid = luaL_checkint(L, 2);
  int gid = luaL_checkint(L, 3);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fchown(loop, &req, fd, uid, gid, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fchown(loop, req, fd, uid, gid, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_fstat(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fstat(loop, &req, fd, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fstat(loop, req, fd, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_fsync(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_fsync(loop, &req, fd, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_fsync(loop, req, fd, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_ftruncate(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  long len = luaL_optlong(L, 2, 0);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_ftruncate(loop, &req, fd, len, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_ftruncate(loop, req, fd, len, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_futime(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  double atime = luaL_checknumber(L, 2);
  double mtime = luaL_checknumber(L, 3);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_futime(loop, &req, fd, atime, mtime, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_futime(loop, req, fd, atime, mtime, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_link(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  const char *new_path = luaL_checkstring(L, 2);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_link(loop, &req, path, new_path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_link(loop, req, path, new_path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_lstat(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_lstat(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_lstat(loop, req, path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_mkdir(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int mode = fs_optmode(L, 2, 0777);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_mkdir(loop, &req, path, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_mkdir(loop, req, path, mode, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_open(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  int flags = fs_checkflags(L, 2);
  int mode = fs_optmode(L, 3, 0666);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_open(loop, &req, path, flags, mode, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_open(loop, req, path, flags, mode, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_readlink(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_readlink(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_readlink(loop, req, path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_rename(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *old_path = luaL_checkstring(L, 1);
  const char *new_path = luaL_checkstring(L, 2);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_rename(loop, &req, old_path, new_path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_rename(loop, req, old_path, new_path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_rmdir(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_rmdir(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_rmdir(loop, req, path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_stat(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_stat(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_stat(loop, req, path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_symlink(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  const char *new_path = luaL_checkstring(L, 2);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_symlink(loop, &req, path, new_path, 0, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_symlink(loop, req, path, new_path, 0, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_unlink(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, path, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_unlink(loop, req, path, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_utime(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  double atime = luaL_checknumber(L, 2);
  double mtime = luaL_checknumber(L, 3);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_utime(loop, &req, path, atime, mtime, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_utime(loop, req, path, atime, mtime, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_read(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  uv_buf_t *buf = luv_checkbuf(L, 2);
  char *p = buf->base;
  int buf_pos = luaL_optint(L, 3, 1);
  int length = luaL_optint(L, 4, buf->len - buf_pos + 1);
  long file_offset = luaL_optlong(L, 5, -1);
  luv_argcheckindex(L, 3, buf_pos, 1, buf->len);
  luv_argcheckindex(L, 4, length, 0, buf->len - buf_pos + 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_read(loop, &req, fd, (void *)&p[buf_pos - 1], length, file_offset,
        NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_read(loop, req, fd, (void *)&p[buf_pos - 1], length,
        file_offset, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_readdir(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  const char *path = luaL_checkstring(L, 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_readdir(loop, &req, path, 0, NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_readdir(loop, req, path, 0, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static int fs_write(lua_State *L) {
  uv_loop_t *loop = luv_loop(L);
  int fd = luaL_checkint(L, 1);
  uv_buf_t buf = luv_checkbuforstr(L, 2);
  int buf_pos = luaL_optint(L, 3, 1);
  int buflen = (int)buf.len;
  char *p = buf.base;
  int length = luaL_optint(L, 4, buflen - buf_pos + 1);
  long file_offset = luaL_optlong(L, 5, -1);
  luv_argcheckindex(L, 3, buf_pos, 1, (int)buflen);
  luv_argcheckindex(L, 4, length, 0, (int)buflen - buf_pos + 1);
  if (luvL_is_in_mainthread(L)) {
    uv_fs_t req;
    uv_fs_write(loop, &req, fd, (void *)&p[buf_pos - 1], length, file_offset,
        NULL);
    return fs_push_common_results(L, &req);
  } else {
    uv_fs_t *req = fs_alloc_req(L);
    int r = uv_fs_write(loop, req, fd, (void *)&p[buf_pos - 1], length,
        file_offset, fs_common_callback);
    return fs_yield_or_error(req, r);
  }
}

static const struct luaL_Reg fs_functions[] = {
  { "chmod", fs_chmod },
  { "chown", fs_chown },
  { "close", fs_close },
  { "exists", fs_exists },
  { "fchmod", fs_fchmod },
  { "fchown", fs_fchown },
  { "fstat", fs_fstat },
  { "fsync", fs_fsync },
  { "ftruncate", fs_ftruncate },
  { "futime", fs_futime },
  { "link", fs_link },
  { "lstat", fs_lstat },
  { "mkdir", fs_mkdir },
  { "open", fs_open },
  { "read", fs_read },
  { "readdir", fs_readdir },
  { "readlink", fs_readlink },
  { "rename", fs_rename },
  { "rmdir", fs_rmdir },
  { "stat", fs_stat },
  { "symlink", fs_symlink },
  { "unlink", fs_unlink },
  { "utime", fs_utime },
  { "write", fs_write },
  { NULL, NULL }
};

int luaopen_yaluv_fs(lua_State *L) {
  lua_createtable(L, 0, ARRAY_SIZE(fs_functions) - 1);
  luaL_register(L, NULL, fs_functions);

  luaL_newmetatable(L, LUV_FS_STAT_MTBL_NAME);
  luaL_register(L, NULL, fs_stat_methods);
  lua_setfield(L, -1, "__index");

  lua_setfield(L, -2, "fs");
  return 1;
}
