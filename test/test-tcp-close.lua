local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['tcp.close'] = function(test)
  coroutine.wrap(function()
    local handle = uv.Tcp.new()
    handle:bind(uv.ip4addr('127.0.0.1', TEST_PORT))
    handle:listen(128, function(server)
      test.ok(true)
    end)
    handle:unref()
  end)()
  
  coroutine.wrap(function()
    local NUM_WRITE_REQS = 32

    local handle = uv.Tcp.new()
    handle:connect(uv.ip4addr('127.0.0.1', TEST_PORT))

    for i = 1, NUM_WRITE_REQS do
      handle:write({"PING"})
    end

    handle:close()
    test.ok(true)
  end)()

  uv.run()
  test.done()
end

return exports
