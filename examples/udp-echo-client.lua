local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local addr = uv.SockAddrV4.new('127.0.0.1', TEST_PORT)
  local handle = uv.Udp.new()

  handle:startRecv()

  handle:send({"PING", "world"}, addr)
  print('udp_client sent {"PING", "world"}')

--[[
  local nread, buf, addr = handle:recv()
  print("udp_client recv nread=", nread)
  if nread and nread > 0 then
    print("udp_client recv msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
  end

  handle:send({"hey"}, addr)
  print('udp_client sent {"hey"}')

  nread, buf, addr = handle:recv()
  print("udp_client recv nread=", nread)
  if nread and nread > 0 then
    print("udp_client recv msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
  end

  handle:stopRecv()
--]]
  handle:close()
end)()

uv.run()
