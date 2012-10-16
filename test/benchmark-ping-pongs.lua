local uv = require 'couv'

local MSG = "PING\n"

local startTime = uv.hrtime()

local co = coroutine.create(function()
  local ok, err = pcall(function()
    local handle = uv.tcp_create()
    uv.tcp_bind(handle, uv.ip4addr('0.0.0.0', 0))
    uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', 9123))
    uv.write(handle, {MSG})
    uv.read_start(handle)

    local pongCount = 0
    repeat
      local nread, buf = uv.read(handle)
      if nread > 0 then
        if buf:toString(1, nread) ~= MSG then
          error(string.format("got wrong answer %s", buf:toString(1, nread)))
        end
        pongCount = pongCount + 1
        uv.write(handle, {MSG})
      end
    until uv.hrtime() - startTime > 5
    uv.close(handle)
    print(string.format("ping_pongs: %d roundtrips/s", pongCount))
  end)
  if not ok then
    print("err=", err)
  end
end)
coroutine.resume(co)

uv.run()
