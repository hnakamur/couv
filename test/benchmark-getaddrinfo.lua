local uv = require 'couv'

local CONCURRENT_COUNT = 1
-- TODO: Fix segfault when CONCURRENT_COUNT > 1
-- local CONCURRENT_COUNT = 10

local TOTAL_CALLS = 10000

local callsInitiated = 0
local callsCompleted = 0
local name = 'localhost'

local startTime = uv.now()

for i = 1, CONCURRENT_COUNT do
  coroutine.wrap(function()
    while callsInitiated < TOTAL_CALLS do
      callsInitiated = callsInitiated + 1
      local addresses = uv.getaddrinfo(name, nil, nil)
      callsCompleted = callsCompleted + 1
    end
  end)()
end

uv.run()
local endTime = uv.now()
assert(callsInitiated == TOTAL_CALLS)
assert(callsCompleted == TOTAL_CALLS)
print(string.format("getaddrinfo: %.0f reqs/s",
    callsCompleted / (endTime - startTime) * 1000))
