local uv = {}
local native = require 'couv_native'

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

uv.Buffer = native.Buffer
uv.Handle = native.Handle
uv.Pipe = native.Pipe
uv.Process = native.Process
uv.Stream = native.Stream
uv.SockAddr = native.SockAddr
uv.SockAddrV4 = native.SockAddrV4
uv.SockAddrV6 = native.SockAddrV6
uv.Tcp = native.Tcp
uv.Timer = native.Timer
uv.Tty = native.Tty
uv.Udp = native.Udp

-- core
uv.chdir = native.chdir
uv.cwd = native.cwd
uv.exepath = native.exepath
uv.getaddrinfo = function(...)
  return error1(native.getaddrinfo(...))
end
uv.getFreeMemory = native.getFreeMemory
uv.getTotalMemory = native.getTotalMemory
uv.interfaceAddresses = native.interfaceAddresses
uv.kill = native.kill
uv.loadavg = native.loadavg
uv.hrtime = native.hrtime
uv.residentSetMemory = native.residentSetMemory
uv.sleep = native.sleep
uv.uptime = native.uptime
uv.updateTime = native.updateTime

-- loop
uv.defaultLoop = native.defaultLoop
uv.getLoop = native.getLoop
uv.now = native.now
uv.run = native.run
uv.runOnce = native.runOnce
uv.setLoop = native.setLoop


-- fs
uv.fs = {}
uv.fs.exists = native.fs.exists

-- no return value functions

uv.fs.chmod = function(...)
  return error0(native.fs.chmod(...))
end

uv.fs.chown = function(...)
  return error0(native.fs.chown(...))
end

uv.fs.close = function(...)
  return error0(native.fs.close(...))
end

uv.fs.fchmod = function(...)
  return error0(native.fs.fchmod(...))
end

uv.fs.fchown = function(...)
  return error0(native.fs.fchown(...))
end

uv.fs.fsync = function(...)
  return error0(native.fs.fsync(...))
end

uv.fs.ftruncate = function(...)
  return error0(native.fs.ftruncate(...))
end

uv.fs.futime = function(...)
  return error0(native.fs.futime(...))
end

uv.fs.link = function(...)
  return error0(native.fs.link(...))
end

uv.fs.mkdir = function(...)
  return error0(native.fs.mkdir(...))
end

uv.fs.rename = function(...)
  return error0(native.fs.rename(...))
end

uv.fs.rmdir = function(...)
  return error0(native.fs.rmdir(...))
end

uv.fs.symlink = function(...)
  return error0(native.fs.symlink(...))
end

uv.fs.unlink = function(...)
  return error0(native.fs.unlink(...))
end

uv.fs.utime = function(...)
  return error0(native.fs.utime(...))
end


-- one return value functions

uv.fs.open = function(...)
  return error1(native.fs.open(...))
end

uv.fs.read = function(...)
  return error1(native.fs.read(...))
end

uv.fs.write = function(...)
  return error1(native.fs.write(...))
end

uv.fs.lstat = function(...)
  return error1(native.fs.lstat(...))
end

uv.fs.fstat = function(...)
  return error1(native.fs.fstat(...))
end

uv.fs.stat = function(...)
  return error1(native.fs.stat(...))
end

uv.fs.readlink = function(...)
  return error1(native.fs.readlink(...))
end

uv.fs.readdir = function(...)
  return error1(native.fs.readdir(...))
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
