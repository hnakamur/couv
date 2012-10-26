local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['udp.options'] = function(test)
  coroutine.wrap(function()
    local handle = uv.Udp.new()

    -- don't keep the loop alive
    handle:unref()

    handle:bind(uv.ip4addr('0.0.0.0', TEST_PORT))
    test.ok(true)

    handle:setBroadcast(true)
    test.ok(true)
    handle:setBroadcast(true)
    test.ok(true)
    handle:setBroadcast(false)
    test.ok(true)
    handle:setBroadcast(false)
    test.ok(true)

    for i = 1, 255 do
      handle:setTtl(i)
      test.ok(true)
    end

    local invalidTTLs = {-1, 0, 256}
    for _, ttl in ipairs(invalidTTLs) do
      local ok, err = pcall(handle.setTtl, handle, ttl)
      test.ok(not ok)
      test.equal(string.sub(err, -#'EINVAL'), 'EINVAL')
    end

    handle:setMulticastLoop(true)
    test.ok(true)
    handle:setMulticastLoop(true)
    test.ok(true)
    handle:setMulticastLoop(false)
    test.ok(true)
    handle:setMulticastLoop(false)
    test.ok(true)

    -- values 0-255 should work
    for i = 0, 255 do
      handle:setMulticastTtl(i)
      test.ok(true)
    end

    -- anything >255 should fail
    local ok, err = pcall(handle.setMulticastTtl, handle, 256)
    test.ok(not ok)
    test.equal(string.sub(err, -#'EINVAL'), 'EINVAL')
    -- don't test ttl=-1, it's a valid value on some platforms

    handle:close()
  end)()

  uv.run()
  test.done()
end

return exports
