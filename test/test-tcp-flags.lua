local uv = require 'couv'

local exports = {}

exports['tcp.flags'] = function(test)
  coroutine.wrap(function()
    local handle = uv.tcp_create()
    test.ok(true)

    uv.tcp_nodelay(handle, true)
    test.ok(true)

    uv.tcp_keepalive(handle, true, 60)
    test.ok(true)

    uv.close(handle)
    test.ok(true)
  end)()

  uv.run()
  test.done()
end

return exports
