local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local handle = uv.tcp_create()
  uv.tcp_connect(handle, uv.ip6addr('::1', TEST_PORT))
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

  uv.write(handle, {"QUIT"})

  uv.read_stop(handle)
  uv.close(handle)
end)()

uv.run()
