local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['tcp.close'] = function(test)
  coroutine.wrap(function()
    local handle = uv.tcp_create()
    uv.tcp_bind(handle, uv.ip4addr('127.0.0.1', TEST_PORT))
    uv.listen(handle, 128, function(server)
      test.ok(true)
    end)
    uv.unref(handle)
  end)()
  
  coroutine.wrap(function()
    local NUM_WRITE_REQS = 32

    local handle = uv.tcp_create()
    uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', TEST_PORT))

    for i = 1, NUM_WRITE_REQS do
      uv.write(handle, {"PING"})
    end

    uv.close(handle)
    test.ok(true)
  end)()

  uv.run()
  test.done()
end

return exports
