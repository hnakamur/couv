local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['udp.multicast_join'] = function(test)
  coroutine.wrap(function()
    local server = uv.Udp.new()

    local client = uv.Udp.new()
    local clientAddr = uv.SockAddrV4.new('127.0.0.1', TEST_PORT)
    client:bind(clientAddr)

    client:setMembership('239.255.0.1', nil, uv.Udp.JOIN_GROUP);
    client:startRecv()

    server:send({'PING'}, clientAddr)

    local nread, buf, addr = client:recv()
    test.ok(nread > 0)
    local msg = buf:toString(1, nread)
    test.equal(msg, 'PING')

    client:close()
    server:close()
  end)()

  uv.run()
  test.done()
end

return exports
