local uv = require 'couv'

local PIPENAME = "/tmp/couv-test-sock"

coroutine.wrap(function()
  local handle = uv.pipe_create()
  uv.pipe_connect(handle, PIPENAME)
  uv.read_start(handle)
  uv.write(handle, {"PING"})
  print('pipe_client sent message {"PING"}')

  local nread, buf = uv.read(handle)
  if nread and nread > 0 then
    print("pipe_client read msg=", buf:toString(1, nread))
  end

  uv.write(handle, {"hello, ", "pipe"})
  print('pipe_client sent message {"hello, ", "pipe"}')

  nread, buf = uv.read(handle)
  if nread and nread > 0 then
    print("pipe_client read msg=", buf:toString(1, nread))
  end

  uv.write(handle, {"QUIT"})
  print('pipe_client sent message {"QUIT"}')

  uv.read_stop(handle)
  uv.close(handle)
end)()

uv.run()
