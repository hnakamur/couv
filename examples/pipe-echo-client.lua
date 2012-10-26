local uv = require 'couv'

local PIPENAME = "/tmp/couv-test-sock"

coroutine.wrap(function()
  local handle = uv.Pipe.new()
  handle:connect(PIPENAME)
  handle:startRead()
  handle:write({"PING"})
  print('pipe_client sent message {"PING"}')

  local nread, buf = handle:read()
  if nread and nread > 0 then
    print("pipe_client read msg=", buf:toString(1, nread))
  end

  handle:write({"hello, ", "pipe"})
  print('pipe_client sent message {"hello, ", "pipe"}')

  nread, buf = handle:read()
  if nread and nread > 0 then
    print("pipe_client read msg=", buf:toString(1, nread))
  end

  handle:write({"QUIT"})
  print('pipe_client sent message {"QUIT"}')

  handle:stopRead()
  handle:close()
end)()

uv.run()
