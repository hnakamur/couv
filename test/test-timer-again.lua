local uv = require 'couv'

local exports = {}

exports['timer.again'] = function(test)
  local closeCalled = 0
  local repeat1CbCalled = 0
  local repeat2CbCalled = 0
  local repeat2CbAllowed = false

  local startTime = uv.now()

  -- Verify that it is not possible to uv.Timer:again a never-started timer.
  local dummy = uv.Timer.new()
  test.ok(dummy)

  local ok, err = pcall(dummy.again, dummy)
  test.ok(not ok)
  test.equal(string.sub(err, -#'EINVAL'), 'EINVAL')

  local repeat1 = uv.Timer.new()
  local repeat2 = uv.Timer.new()

  function repeat1Cb(handle)
    test.equal(handle, repeat1)
    test.equal(repeat1:getRepeat(), 50)

    print(string.format("repeat1Cb called after %.1f ms",
        uv.now() - startTime))

    repeat1CbCalled = repeat1CbCalled + 1
    repeat2:again()

    if uv.now() >= startTime + 500 then
      handle:close()
      closeCalled = closeCalled + 1
      -- We're not calling uv.Timer:again on repeat2 any more, so after this
      -- timer2Cb is expected.
      repeat2CbAllowed = true
    end
  end

  function repeat2Cb(handle)
    test.equal(handle, repeat2)
    test.ok(repeat2CbAllowed)
    print(string.format("repeat2Cb called after %.1f ms", uv.now() - startTime))
    repeat2CbCalled = repeat2CbCalled + 1
    if repeat2:getRepeat() == 0 then
      test.ok(not handle:isActive())
      handle:close()
      closeCalled = closeCalled + 1
      return
    end
    print(string.format("repeat2:getRepeat %d ms", repeat2:getRepeat()))
    test.equal(repeat2:getRepeat(), 100)

    -- This shouldn't take effect immediately.
    repeat2:setRepeat(0)
  end

  repeat1:start(repeat1Cb, 50, 0)
  test.equal(repeat1:getRepeat(), 0)
  repeat1:setRepeat(50)

  repeat2:start(repeat2Cb, 100, 100)
  test.equal(repeat2:getRepeat(), 100)

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
