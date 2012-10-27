local uv = require 'couv'

local exports = {}

exports['udp.send_and_recv'] = function(test)
  coroutine.wrap(function()
    local handle = uv.Udp.new()
    handle:bind(uv.SockAddrV4.new('127.0.0.1', 62001))
    handle:startRecv()
    local nread, buf, addr = handle:recv()
    test.equal(nread, #'helloworld')
    test.equal(buf:toString(1, nread), 'helloworld')
    test.equal(addr:host(), '127.0.0.1')
    test.is_number(addr:port())
    handle:stopRecv()
    handle:close()
  end)()
  
  coroutine.wrap(function()
    local handle = uv.Udp.new()
    handle:send({"hello", "world"}, uv.SockAddrV4.new('127.0.0.1', 62001))
    handle:close()
  end)()

  uv.run()
  test.done()
end

--[[
exports['udp.send_and_recv_twice'] = function(test)
  coroutine.wrap(function()
    local handle = uv.Udp.new()
    handle:bind(uv.SockAddrV4.new('127.0.0.1', 62001))
    local nread, buf, addr = handle:recv()
    test.equal(nread, #'helloworld')
    test.equal(buf:toString(1, nread), 'helloworld')
    test.equal(addr:host(), '127.0.0.1')
    test.is_number(addr:port())

    nread, buf, addr = handle:recv()
    test.equal(nread, #'hi')
    test.equal(buf:toString(1, nread), 'hi')
    test.equal(addr:host(), '127.0.0.1')
    test.is_number(addr:port())

    handle:close()
  end)()
  
  coroutine.wrap(function()
    local handle = uv.Udp.new()
    handle:send({"hello", "world"}, uv.SockAddrV4.new('127.0.0.1', 62001))
    handle:send({"hi"}, uv.SockAddrV4.new('127.0.0.1', 62001))
    handle:close()
  end)()

  uv.run()
  test.done()
end
]]

return exports
