local uv = require 'yaluv'
local loop = uv.loop

local co = coroutine.create(function()
  local handle = uv.udp_create()

  uv.udp_recv_start(handle)
  uv.udp_send(handle, uv.ip4addr('127.0.0.1', 9123), {"PING", "world"})

  local nread, buf, addr = uv.udp_recv(handle)
  print("udp_client#2 nread=", nread)
  if nread and nread > 0 then
    print("udp_client#3 msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
  end

--[[
  uv.udp_send(handle, uv.ip4addr('127.0.0.1', 9123), {"hey"})

  nread, buf, addr = uv.udp_recv(handle)
  print("udp_client#4 nread=", nread)
  if nread and nread > 0 then
    print("udp_client#5 msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
  end
--]]

  uv.udp_recv_stop(handle)
  uv.close(handle)
end)
coroutine.resume(co)

loop.get():run()
