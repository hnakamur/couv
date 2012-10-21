local uv = require 'couv'

local exports = {}

exports['tty'] = function(test)
  local ttyInFd = uv.tty_open_fd(0)
  local ttyOutFd = uv.tty_open_fd(1)
  test.ok(ttyInFd >= 0)
  test.ok(ttyOutFd >= 0)

  test.equal(uv.guess_handle(-1), uv.UNKNOWN_HANDLE)
  test.equal(uv.guess_handle(ttyInFd), uv.TTY)
  test.equal(uv.guess_handle(ttyOutFd), uv.TTY)

  local ttyIn = uv.tty_create(ttyInFd, 1)
  local ttyOut = uv.tty_create(ttyOutFd, 2)

  local width, height = uv.tty_get_winsize(ttyOut)
  print(string.format("width=%d height=%d", width, height))

  -- Is it a safe assumption that most people have terminals larger than 10x10?
  test.ok(width > 10)
  test.ok(height > 10)

  -- Turn on raw mode.
  uv.tty_set_mode(ttyIn, 1)

  -- Turn off raw mode.
  uv.tty_set_mode(ttyIn, 0)

  -- TODO: check the actual mode!

  uv.close(ttyIn)
  uv.close(ttyOut)

  uv.run()
  test.done()
end

return exports
