local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['udp.options'] = function(test)
  coroutine.wrap(function()
    local handle = uv.udp_create()

    -- don't keep the loop alive
    uv.unref(handle)

    uv.udp_bind(handle, uv.ip4addr('0.0.0.0', TEST_PORT))
    test.ok(true)

    uv.udp_set_broadcast(handle, true)
    test.ok(true)
    uv.udp_set_broadcast(handle, true)
    test.ok(true)
    uv.udp_set_broadcast(handle, false)
    test.ok(true)
    uv.udp_set_broadcast(handle, false)
    test.ok(true)

    for i = 1, 255 do
      uv.udp_set_ttl(handle, i)
      test.ok(true)
    end

    local invalidTTLs = {-1, 0, 256}
    for _, ttl in ipairs(invalidTTLs) do
      local ok, err = pcall(uv.udp_set_ttl, handle, ttl)
      test.ok(not ok)
      test.equal(string.sub(err, -#'EINVAL'), 'EINVAL')
    end

    uv.udp_set_multicast_loop(handle, true)
    test.ok(true)
    uv.udp_set_multicast_loop(handle, true)
    test.ok(true)
    uv.udp_set_multicast_loop(handle, false)
    test.ok(true)
    uv.udp_set_multicast_loop(handle, false)
    test.ok(true)

    -- values 0-255 should work
    for i = 0, 255 do
      uv.udp_set_multicast_ttl(handle, i)
      test.ok(true)
    end

    -- anything >255 should fail
    local ok, err = pcall(uv.udp_set_multicast_ttl, handle, 256)
    test.ok(not ok)
    test.equal(string.sub(err, -#'EINVAL'), 'EINVAL')
    -- don't test ttl=-1, it's a valid value on some platforms

    uv.close(handle)
  end)()

  uv.run()
  test.done()
end

return exports
