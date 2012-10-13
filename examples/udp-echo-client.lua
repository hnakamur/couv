local uv = require 'yaluv'
local loop = uv.loop

local co = coroutine.create(function()
  local handle = uv.udp_create()
  uv.udp_send(handle, uv.ip4addr('127.0.0.1', 9123), {"PING", "world"})
  uv.udp_send(handle, uv.ip4addr('127.0.0.1', 9123), {"hey"})
  uv.close(handle)
end)
coroutine.resume(co)

loop.get():run()
