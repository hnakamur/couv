local native = require 'couv_native'

local uv = {}

uv.Buffer = native.Buffer

uv.accept = native.accept
uv.listen = native.listen
uv.close = native.close
uv.read_start = native.read_start
uv.read_stop = native.read_stop
uv.write = native.write

uv.ip4addr = native.ip4addr
uv.ip6addr = native.ip6addr

uv.default_loop = native.default_loop
uv.get_loop = native.get_loop
uv.run = native.run
uv.run_once = native.run_once
uv.set_loop = native.set_loop

uv.tcp_bind = native.tcp_bind
uv.tcp_create = native.tcp_create

uv.udp_bind = native.udp_bind
uv.udp_create = native.udp_create
uv.udp_open = native.udp_open
uv.udp_recv_start = native.udp_recv_start
uv.udp_recv_stop = native.udp_recv_stop

uv.fs_exists = native.fs_exists

-- utility functions

local function error0(err)
  if err then
    error(err)
  end
end

local function error1(ret, err)
  if err then
    error(err)
  end
  return ret
end

-- no return value functions

uv.fs_chmod = function(...)
  return error0(native.fs_chmod(...))
end

uv.fs_chown = function(...)
  return error0(native.fs_chown(...))
end

uv.fs_close = function(...)
  return error0(native.fs_close(...))
end

uv.fs_fchmod = function(...)
  return error0(native.fs_fchmod(...))
end

uv.fs_fchown = function(...)
  return error0(native.fs_fchown(...))
end

uv.fs_fsync = function(...)
  return error0(native.fs_fsync(...))
end

uv.fs_ftruncate = function(...)
  return error0(native.fs_ftruncate(...))
end

uv.fs_futime = function(...)
  return error0(native.fs_futime(...))
end

uv.fs_link = function(...)
  return error0(native.fs_link(...))
end

uv.fs_mkdir = function(...)
  return error0(native.fs_mkdir(...))
end

uv.fs_rename = function(...)
  return error0(native.fs_rename(...))
end

uv.fs_rmdir = function(...)
  return error0(native.fs_rmdir(...))
end

uv.fs_symlink = function(...)
  return error0(native.fs_symlink(...))
end

uv.fs_unlink = function(...)
  return error0(native.fs_unlink(...))
end

uv.fs_utime = function(...)
  return error0(native.fs_utime(...))
end

uv.tcp_connect = function(...)
  return error0(native.tcp_connect(...))
end

uv.udp_send = function(...)
  return error0(native.udp_send(...))
end

-- one return value functions

uv.fs_open = function(...)
  return error1(native.fs_open(...))
end

uv.fs_read = function(...)
  return error1(native.fs_read(...))
end

uv.fs_write = function(...)
  return error1(native.fs_write(...))
end

uv.fs_lstat = function(...)
  return error1(native.fs_lstat(...))
end

uv.fs_fstat = function(...)
  return error1(native.fs_fstat(...))
end

uv.fs_stat = function(...)
  return error1(native.fs_stat(...))
end

uv.fs_readlink = function(...)
  return error1(native.fs_readlink(...))
end

uv.fs_readdir = function(...)
  return error1(native.fs_readdir(...))
end

-- wrapper functions to circumvent the limitation that C function cannot yield
-- by calling them inside a loop in lua.

uv.udp_recv = function(handle)
  local nread, buf, addr
  repeat
    nread, buf, addr = native.udp_prim_recv(handle)
  until nread
  return nread, buf, addr
end

uv.read = function(handle)
  local nread, buf
  repeat
    nread, buf, addr = native.prim_read(handle)
  until nread
  return nread, buf
end

return uv
