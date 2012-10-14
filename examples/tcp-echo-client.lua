local uv = require 'yaluv'
local loop = uv.loop

local co = coroutine.create(function()
  local handle = uv.tcp_create()
  print("tcp_client#1")
  uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', 9123))
  print("tcp_client#2")
  uv.read_start(handle)
  print("tcp_client#3")
  uv.write(handle, {"PING"})
  print("tcp_client#4")

  local nread, buf = uv.read(handle)
  print("tcp_client#4 nread=", nread)
  if nread and nread > 0 then
    print("tcp_client#5 msg=", buf:toString(1, nread))
  end

  uv.write(handle, {"hello, ", "tcp"})
  print("tcp_client#6")

  nread, buf = uv.read(handle)
  print("tcp_client#7 nread=", nread)
  if nread and nread > 0 then
    print("tcp_client#8 msg=", buf:toString(1, nread))
  end

  uv.read_stop(handle)
  print("tcp_client#9")
  uv.close(handle)
  print("tcp_client#10")
end)
coroutine.resume(co)

loop.get():run()
