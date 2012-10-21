local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['udp.multicast-ttl'] = function(test)
  coroutine.wrap(function()
    local addr = uv.ip4addr('239.255.0.1', TEST_PORT)

    local server = uv.udp_create()
    test.ok(server)

    uv.udp_bind(server, uv.ip4addr('0.0.0.0', 0))
    test.ok(true)

    uv.udp_set_multicast_ttl(server, 32)
    test.ok(true)

    uv.udp_send(server, {"PING"}, addr)
    test.ok(true)

    uv.close(server)
  end)()

  uv.run()
  test.done()
end

return exports
