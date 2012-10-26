local uv = {}
local native = require 'couv_native'

uv.Buffer = native.Buffer
uv.Handle = native.Handle
uv.Pipe = native.Pipe
uv.Process = native.Process
uv.Stream = native.Stream
uv.Tcp = native.Tcp
uv.Timer = native.Timer
uv.Tty = native.Tty
uv.Udp = native.Udp

-- fs
uv.fs_exists = native.fs_exists

-- core
uv.chdir = native.chdir
uv.cwd = native.cwd
uv.exepath = native.exepath
uv.get_free_memory = native.get_free_memory
uv.get_total_memory = native.get_total_memory
uv.kill = native.kill
uv.loadavg = native.loadavg
uv.hrtime = native.hrtime
uv.resident_set_memory = native.resident_set_memory
uv.uptime = native.uptime
uv.update_time = native.update_time

-- ipaddr
uv.ip4addr = native.ip4addr
uv.ip6addr = native.ip6addr

-- loop
uv.default_loop = native.default_loop
uv.get_loop = native.get_loop
uv.now = native.now
uv.run = native.run
uv.run_once = native.run_once
uv.set_loop = native.set_loop


-- utility functions

local function error0(err)
  if err then
    error(err, 2)
  end
end

local function error1(ret, err)
  if err then
    error(err, 2)
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



native._Handle.close = function(...)
  return error0(native._Handle._close(...))
end


native._Stream.read = function(handle)
  local nread, buf
  repeat
    nread, buf = native._Stream._read(handle)
  until nread
  return nread, buf
end

native._Stream.shutdown = function(...)
  return error0(native._Stream._shutdown(...))
end

native._Stream.write = function(...)
  return error0(native._Stream._write(...))
end


native._Tcp.connect = function(...)
  return error0(native._Tcp._connect(...))
end


native._Pipe.connect = function(...)
  return error0(native._Pipe._connect(...))
end

native._Pipe.read2 = function(...)
  local nread, buf, pending
  repeat
    nread, buf, pending = native._Pipe._read2(...)
  until nread
  return nread, buf, pending
end


native._Udp.recv = function(handle)
  local nread, buf, addr
  repeat
    nread, buf, addr = native._Udp._recv(handle)
  until nread
  return nread, buf, addr
end

native._Udp.send = function(...)
  return error0(native._Udp._send(...))
end

-- for debug
uv.pt = function(t)
  if type(t) ~= 'table' then
    print('not table', t)
    return
  end
  print(t, '{')
  for k, v in pairs(t) do
    print(k, v)
  end
  print('}')
end

return uv
