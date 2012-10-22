local native = require 'couv_native'

local uv = {}

uv.Buffer = native.Buffer

-- handle types
uv.UNKNOWN_HANDLE = native.UNKNOWN_HANDLE
uv.ASYNC = native.ASYNC
uv.CHECK = native.CHECK
uv.FS_EVENT = native.FS_EVENT
uv.FS_POLL = native.FS_POLL
uv.HANDLE = native.HANDLE
uv.IDLE = native.IDLE
uv.NAMED_PIPE = native.NAMED_PIPE
uv.POLL = native.POLL
uv.PREPARE = native.PREPARE
uv.PROCESS = native.PROCESS
uv.STREAM = native.STREAM
uv.TCP = native.TCP
uv.TIMER = native.TIMER
uv.TTY = native.TTY
uv.UDP = native.UDP
uv.SIGNAL = native.SIGNAL
uv.FILE = native.FILE
uv.HANDLE_TYPE_MAX = native.HANDLE_TYPE_MAX

uv.JOIN_GROUP = native.JOIN_GROUP
uv.LEAVE_GROUP = native.LEAVE_GROUP

-- fs_open mode flags
uv.S_IREAD = native.S_IREAD
uv.S_IWRITE = native.S_IWRITE

-- process flags
uv.PROCESS_SETUID = native.PROCESS_SETUID
uv.PROCESS_SETGID = native.PROCESS_SETGID
uv.PROCESS_WINDOWS_VERBATIM_ARGUMENTS = native.PROCESS_WINDOWS_VERBATIM_ARGUMENTS
uv.PROCESS_DETACHED = native.PROCESS_DETACHED

-- stdio flags
uv.IGNORE = native.IGNORE
uv.CREATE_PIPE = native.CREATE_PIPE
uv.INHERIT_FD = native.INHERIT_FD
uv.INHERIT_STREAM = native.INHERIT_STREAM
uv.READABLE_PIPE = native.READABLE_PIPE
uv.WRITABLE_PIPE = native.WRITABLE_PIPE

-- stream
uv.accept = native.accept
uv.listen = native.listen
uv.close = native.close

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

-- handle
uv.get_write_queue_size = native.get_write_queue_size
uv.is_active = native.is_active
uv.is_closing = native.is_closing
uv.is_readable = native.is_readable
uv.is_writable = native.is_writable
uv.read_start = native.read_start
uv.read_stop = native.read_stop
uv.ref = native.ref
uv.unref = native.unref
uv.update_time = native.update_time
uv.write = native.write

-- process
uv.spawn = native.spawn
uv.get_pid = native.get_pid
uv.process_kill = native.process_kill

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

uv.guess_handle = native.guess_handle

uv.tcp_bind = native.tcp_bind
uv.tcp_create = native.tcp_create
uv.tcp_open = native.tcp_open
uv.tcp_keepalive = native.tcp_keepalive
uv.tcp_nodelay = native.tcp_nodelay
uv.tcp_simultaneous_accepts = native.tcp_simultaneous_accepts

uv.timer_again = native.timer_again
uv.timer_get_repeat = native.timer_get_repeat
uv.timer_set_repeat = native.timer_set_repeat
uv.timer_start = native.timer_start
uv.timer_stop = native.timer_stop

uv.tty_create = native.tty_create
uv.tty_get_winsize = native.tty_get_winsize
uv.tty_open_fd = native.tty_open_fd
uv.tty_reset_mode = native.tty_reset_mode
uv.tty_set_mode = native.tty_set_mode

uv.udp_bind = native.udp_bind
uv.udp_create = native.udp_create
uv.udp_open = native.udp_open
uv.udp_recv_start = native.udp_recv_start
uv.udp_recv_stop = native.udp_recv_stop
uv.udp_set_broadcast = native.udp_set_broadcast
uv.udp_set_membership = native.udp_set_membership
uv.udp_set_multicast_loop = native.udp_set_multicast_loop
uv.udp_set_multicast_ttl = native.udp_set_multicast_ttl
uv.udp_set_ttl = native.udp_set_ttl


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

uv.pipe_bind = function(...)
  return error0(native.pipe_bind(...))
end

uv.pipe_connect = function(...)
  return error0(native.pipe_connect(...))
end

uv.pipe_open = function(...)
  return error0(native.pipe_open(...))
end

uv.shutdown = function(...)
  return error0(native.shutdown(...))
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

uv.pipe_create = function(...)
  return error1(native.pipe_create(...))
end

uv.timer_create = function(...)
  return error1(native.timer_create(...))
end

uv.tcp_getpeername = function(...)
  return error1(native.tcp_getpeername(...))
end

uv.tcp_getsockname = function(...)
  return error1(native.tcp_getsockname(...))
end

uv.udp_getsockname = function(...)
  return error1(native.udp_getsockname(...))
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
    nread, buf = native.prim_read(handle)
  until nread
  return nread, buf
end

uv.read2 = function(...)
  local nread, buf, pending
  repeat
    nread, buf, pending = native.prim_read2(...)
  until nread
  return nread, buf, pending
end

return uv
