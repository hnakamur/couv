local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['udp.multicast-ttl'] = function(test)
  coroutine.wrap(function()
    local addr = uv.SockAddrV4.new('239.255.0.1', TEST_PORT)

    local server = uv.Udp.new()
    test.ok(server)

    server:bind(uv.SockAddrV4.new('0.0.0.0', 0))
    test.ok(true)

    server:setMulticastTtl(32)
    test.ok(true)

    server:send({"PING"}, addr)
    test.ok(true)

    server:close()
  end)()

  uv.run()
  test.done()
end

return exports
