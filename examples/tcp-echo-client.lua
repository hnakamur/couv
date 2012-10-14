local uv = require 'yaluv'

local co = coroutine.create(function()
  local handle = uv.tcp_create()
  uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', 9123))
  uv.read_start(handle)
  uv.write(handle, {"PING"})
  print('tcp_client send message {"PING"}')

  local nread, buf = uv.read(handle)
  if nread and nread > 0 then
    print("tcp_client read msg=", buf:toString(1, nread))
  end

  uv.write(handle, {"hello, ", "tcp"})
  print('tcp_client send message {"hello, ", "tcp"}')

  nread, buf = uv.read(handle)
  if nread and nread > 0 then
    print("tcp_client read msg=", buf:toString(1, nread))
  end

  uv.read_stop(handle)
  uv.close(handle)
end)
coroutine.resume(co)

uv.run()
