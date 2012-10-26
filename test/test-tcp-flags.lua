local uv = require 'couv'

local exports = {}

exports['tcp.flags'] = function(test)
  coroutine.wrap(function()
    local handle = uv.Tcp.new()
    test.ok(true)

    handle:nodelay(true)
    test.ok(true)

    handle:keepalive(true, 60)
    test.ok(true)

    handle:close()
    test.ok(true)
  end)()

  uv.run()
  test.done()
end

return exports
