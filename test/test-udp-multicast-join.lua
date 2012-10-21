local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['udp.multicast_join'] = function(test)
  local clientAddr = uv.ip4addr('127.0.0.1', TEST_PORT)

  coroutine.wrap(function()
    local server = uv.udp_create()
    uv.udp_send(server, {'PING'}, clientAddr)
    uv.close(server)
  end)()

  coroutine.wrap(function()
    local client = uv.udp_create()
    uv.udp_bind(client, clientAddr)

    uv.udp_set_membership(client, '239.255.0.1', nil, uv.JOIN_GROUP);
    uv.udp_recv_start(client)

    local nread, buf, addr = uv.udp_recv(client)
    test.ok(nread > 0)
    local msg = buf:toString(1, nread)
    test.equal(msg, 'PING')

    uv.close(client)
  end)()

  uv.run()
  test.done()
end

return exports
