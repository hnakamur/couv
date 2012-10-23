local uv = require 'couv'

local MSG = "PING\n"

local startTime = uv.hrtime()

function exitCb(process, exitStatus, termSignal)
  uv.close(process)
end
local process = uv.spawn{args={uv.exepath(), 'examples/tcp-echo-server.lua'},
    flags=uv.PROCESS_DETACHED,
    exitCb=exitCb}

function timerCb()
  coroutine.wrap(function()
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
      until uv.hrtime() - startTime > 5e9
      uv.write(handle, {'QUIT'})
      uv.close(handle)
      print(string.format("ping_pongs: %d roundtrips/s", pongCount))
    end)
    if err then
      print(err)
    end
  end)()
end

-- We need to use a timer to avoid an error on OSX.
-- test/benchmark-ping-pongs.lua:19: ECONNREFUSED
local timer = uv.timer_create()
uv.timer_start(timer, timerCb, 10)

uv.run()
