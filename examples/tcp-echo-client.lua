local uv = require 'yaluv'
local loop = uv.loop

coroutine.wrap(function()
  local handle = uv.tcp_create()
  uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', 9123))
  uv.write(handle, {"PING"})
end)()

loop.get():run()
