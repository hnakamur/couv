local uv = require 'yaluv'
local loop = uv.loop

local co = coroutine.create(function()
  local handle = uv.udp_create()
  uv.udp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
  uv.udp_recv_start(handle)

  local nread, buf, addr
  repeat
    nread, buf, addr = uv.udp_recv(handle)
    if nread and nread > 0 then
      uv.udp_send(handle, addr, {buf:toString(1, nread)})
    end
  until nread and nread == 0

  uv.udp_recv_stop(handle)
  uv.udp_close(handle)
end)
coroutine.resume(co)

loop.get():run()
