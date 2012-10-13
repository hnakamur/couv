local uv = require 'yaluv'
local loop = uv.loop
local udp = uv.udp

local exports = {}

exports['udp.send'] = function(test)
  coroutine.wrap(function()
    local handle = udp.udp_create()
    udp.udp_bind(handle, uv.ip4addr('127.0.0.1', 62001))
    udp.udp_send(handle, uv.ip4addr('127.0.0.1', 62002), {"hello", "world"})
    uv.close(handle)

    test.done()
  end)()

  loop.get():run()
end

exports['udp.send-and-recv'] = function(test)
  local server = coroutine.create(function()
    local handle = udp.udp_create()
print("server#1")
    udp.udp_bind(handle, uv.ip4addr('127.0.0.1', 62001))
print("server#2")
    local nread, buf, addr = udp.udp_recv(handle)
    print('nread=', nread, ', buf=', buf:toString(), ', host=', addr:host(),
        ', port=', addr:port())
    uv.udp_close(handle)
print("server#3")
  end)
  coroutine.resume(server)
  
  local client = coroutine.create(function()
    local handle = udp.udp_create()
print("client#1")
    udp.udp_send(handle, uv.ip4addr('127.0.0.1', 62001), {"hello", "world"})
print("client#2")
    uv.close(handle)
print("client#3")
  end)
  coroutine.resume(client)

  loop.get():run()
end

return exports
