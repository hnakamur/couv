local uv = require 'couv'

local exports = {}

exports['tty'] = function(test)
  local ttyInFd = uv.Tty.openFd(0)
  local ttyOutFd = uv.Tty.openFd(1)
  test.ok(ttyInFd >= 0)
  test.ok(ttyOutFd >= 0)

  test.equal(uv.Handle.guess(-1), uv.Handle.UNKNOWN_HANDLE)
  test.equal(uv.Handle.guess(ttyInFd), uv.Handle.TTY)
  test.equal(uv.Handle.guess(ttyOutFd), uv.Handle.TTY)

  local ttyIn = uv.Tty.new(ttyInFd, 1)
  local ttyOut = uv.Tty.new(ttyOutFd, 2)

  local width, height = ttyOut:getWinSize()
  print(string.format("width=%d height=%d", width, height))

  -- Is it a safe assumption that most people have terminals larger than 10x10?
  test.ok(width > 10)
  test.ok(height > 10)

  -- Turn on raw mode.
  ttyIn:setMode(1)

  -- Turn off raw mode.
  ttyIn:setMode(0)

  -- TODO: check the actual mode!

  ttyIn:close()
  ttyOut:close()

  uv.run()
  test.done()
end

return exports
