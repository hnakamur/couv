local uv = require 'yaluv'
local loop = uv.loop

coroutine.wrap(function()
  local handle = uv.tcp_create()
  uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', 9123))
  uv.write(handle, {"PING"})
  local nread, buf = uv.read_start(handle)
  if nread == 0 then
    uv.close(handle)
    return
  end
  print("tcp client nread=", nread, ", received=", buf:toString(1, nread))
  uv.close(handle)
end)()

loop.get():run()
