local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local handle = uv.Tcp.new()
  handle:connect(uv.ip6addr('::1', TEST_PORT))
  handle:startRead()
  handle:write({"PING"})
  print('tcp_client send message {"PING"}')

  local nread, buf = handle:read()
  if nread and nread > 0 then
    print("tcp_client read msg=", buf:toString(1, nread))
  end

  handle:write({"hello, ", "tcp"})
  print('tcp_client send message {"hello, ", "tcp"}')

  nread, buf = handle:read()
  if nread and nread > 0 then
    print("tcp_client read msg=", buf:toString(1, nread))
  end

  handle:write({"QUIT"})

  handle:stopRead()
  handle:close()
end)()

uv.run()
