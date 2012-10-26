local uv = require 'couv'

local NUM_TIMERS = 1000 * 1000
local timerCbCalled = 0

function timerCb(handle)
  timerCbCalled = timerCbCalled + 1
end

local timers = {}
local timeout = 0
for i = 1, NUM_TIMERS do
  if i % 1000 == 0 then
    timeout = timeout + 1
  end
  timers[i] = uv.Timer.new()
  timers[i]:start(timerCb, timeout, 0)
end

local before = uv.hrtime()
uv.run()
local after = uv.hrtime()
assert(timerCbCalled == NUM_TIMERS)
print(string.format("million-timers: %.2f seconds", (after - before) / 1e9))
