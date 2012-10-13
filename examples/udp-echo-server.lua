local uv = require 'yaluv'
local loop = uv.loop
local udp = uv.udp

coroutine.wrap(function()
  local handle = udp.udp_create()
print("server#1")
  udp.udp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
print("server#2")
  udp.udp_recv(handle)
--  local nread, buf, addr = udp.udp_recv(handle)
--  print('nread=', nread, ', buf=', buf:toString(), ', host=', addr:host(),
--      ', port=', addr:port())
--  uv.udp_close(handle)
print("server#3")
end)()

loop.get():run()
