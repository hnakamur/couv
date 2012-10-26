local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local serverAddr = uv.ip6addr('::1', TEST_PORT)
  local handle = uv.Udp.new()

  handle:startRecv()
  print('udp_client after recv_start')

  handle:send({"PING", "world"}, serverAddr)
  -- TODO: Fix to continue from here.
  print('udp_client sent {"PING", "world"}')

--[[
  local nread, buf, addr = handle:recv()
  print("udp_client recv nread=", nread)
  if nread and nread > 0 then
    print("udp_client recv msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
  end


  handle:send({"hey"}, serverAddr)
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
