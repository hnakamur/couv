local uv = require 'yaluv'
local loop = uv.loop
local udp = uv.udp

coroutine.wrap(function()
  local handle = udp.udp_create()
print("client#1")
  udp.udp_send(handle, uv.ip4addr('127.0.0.1', 62001), {"PING"})
print("client#2")
--  uv.close(handle)
--print("client#3")
end)()

loop.get():run()
