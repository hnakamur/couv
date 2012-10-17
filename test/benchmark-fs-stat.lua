local uv = require 'couv'

local NUM_SYNC_REQS = 1e6
local NUM_ASYNC_REQS = 1e5
local MAX_CONCURRENT_REQS = 32

local function warmup(path)
  -- warm up the thread pool
  coroutine.wrap(function()
    local ok, err = pcall(function()
      for i = 1, MAX_CONCURRENT_REQS do
        uv.fs_stat(path)
      end
    end)
    if not ok then
      error(err)
    end
  end)()
  uv.run()

  -- warm up the OS dirent cache
  for i = 1, 16 do
    uv.fs_stat(path)
  end
end

local function syncBench(path)
  local before, after

  before = uv.hrtime()
  for i = 1, NUM_SYNC_REQS do
    uv.fs_stat(path)
  end
  after = uv.hrtime()

  print(string.format("%d stats (sync): %.2fs (%d/s)", NUM_SYNC_REQS,
      after - before, NUM_SYNC_REQS / (after - before)))
end

local function asyncBench(path)
  local before, after

  for i = 1, MAX_CONCURRENT_REQS do
    local count = NUM_ASYNC_REQS
    local before = uv.hrtime()
    for j = 1, i do
      local co = coroutine.create(function()
        local ok, err = pcall(function()
          while count > 0 do
            uv.fs_stat(path)
            count = count - 1
          end
        end)
        if not ok then
          error(err)
        end
      end)
      coroutine.resume(co)
    end
    uv.run()
    local after = uv.hrtime()
    print(string.format("%d stats (%d concurrent): %.2fs (%d/s)",
        NUM_ASYNC_REQS, i, after - before, NUM_ASYNC_REQS / (after - before)))
  end
end

local path = "."
warmup(path)
syncBench(path)
asyncBench(path)
