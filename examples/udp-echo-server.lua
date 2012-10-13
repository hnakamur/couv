local uv = require 'yaluv'
local loop = uv.loop

coroutine.wrap(function()
  local handle = uv.udp_create()
  uv.udp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
  local nread, buf, addr = uv.udp_recv(handle)
  print('server#2.1 received=', buf:toString(1, nread),
      ', host=', addr:host(), ', port=', addr:port())
  nread, buf, addr = uv.udp_recv(handle)
  print('server#2.2 received=', buf:toString(1, nread),
      ', host=', addr:host(), ', port=', addr:port())
  uv.udp_close(handle)
end)()

loop.get():run()
