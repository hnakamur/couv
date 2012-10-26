local uv = require 'couv'

local exports = {}

exports['timer'] = function(test)
  local onceCbCalled = 0

  function onceCb(handle)
    print(string.format("ONCE_CB %d", onceCbCalled))
    test.ok(handle)
    test.ok(not handle:isActive())
    onceCbCalled = onceCbCalled + 1
    handle:close()
  end

  local repeat1CbCalled = 0
  local repeat2CbCalled = 0
  local repeat2CbAllowed = false

  local startTime = uv.now()
  local onceTimers = {}

  -- Let 10 timers time out in 500 ms total.
  for i = 1, 10 do
    onceTimers[i] = uv.Timer.new()
    onceTimers[i]:start(onceCb, i * 50, 0)
  end

  uv.run()
  test.done()
end

return exports
