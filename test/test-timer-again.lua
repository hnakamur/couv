local uv = require 'couv'

local exports = {}

exports['timer.again'] = function(test)
  local closeCalled = 0
  local repeat1CbCalled = 0
  local repeat2CbCalled = 0
  local repeat2CbAllowed = false

  local startTime = uv.now()

  -- Verify that it is not possible to uv.timer_again a never-started timer. */
  local dummy = uv.timer_create()
  test.ok(dummy)

  local ok, err = pcall(uv.timer_again, dummy)
  test.ok(not ok)
  test.equal(string.sub(err, -#'EINVAL'), 'EINVAL')

  local repeat1 = uv.timer_create()
  local repeat2 = uv.timer_create()

  function repeat1Cb(handle)
    test.equal(handle, repeat1)
    test.equal(uv.timer_get_repeat(repeat1), 50)

    print(string.format("repeat1Cb called after %.1f ms",
        uv.now() - startTime))

    repeat1CbCalled = repeat1CbCalled + 1
    uv.timer_again(repeat2)

    if uv.now() >= startTime + 500 then
      uv.close(handle)
      closeCalled = closeCalled + 1
      -- We're not calling uv.timer_again on repeat2 any more, so after this
      -- timer2Cb is expected.
      repeat2CbAllowed = true
    end
  end

  function repeat2Cb(handle)
    test.equal(handle, repeat2)
    test.ok(repeat2CbAllowed)
    print(string.format("repeat2Cb called after %.1f ms", uv.now() - startTime))
    repeat2CbCalled = repeat2CbCalled + 1
    if uv.timer_get_repeat(repeat2) == 0 then
      test.ok(not uv.is_active(handle))
      uv.close(handle)
      closeCalled = closeCalled + 1
    end
    print(string.format("uv.timer_get_repeat %d ms",
        uv.timer_get_repeat(repeat2)))
    test.equal(uv.timer_get_repeat(repeat2), 100)

    -- This shouldn't take effect immediately.
    uv.timer_set_repeat(repeat2, 0)
  end

  uv.timer_start(repeat1, repeat1Cb, 50, 0)
  test.equal(uv.timer_get_repeat(repeat1), 0)
  uv.timer_set_repeat(repeat1, 50)

  uv.timer_start(repeat2, repeat2Cb, 100, 100)
  test.equal(uv.timer_get_repeat(repeat2), 100)

  uv.run()
  test.equal(repeat1CbCalled, 10)
  test.equal(repeat2CbCalled, 2)
  test.equal(closeCalled, 2)
  print(string.format("Test took %.1f ms (expected ~700 ms)",
      uv.now() - startTime))
  test.ok(uv.now() - startTime > 700)
  test.done()
end

return exports
