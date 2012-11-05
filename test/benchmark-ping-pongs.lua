local uv = require 'couv'

local MSG = "PING\n"
local TIME = 5000

local startTime = uv.now()

function exitCb(process, exitStatus, termSignal)
  process:close()
end
local process = uv.Process.spawn{args={uv.exepath(), 'examples/tcp-echo-server.lua'},
    flags=uv.Process.DETACHED,
    exitCb=exitCb}

function timerCb()
  coroutine.wrap(function()
    local ok, err = pcall(function()
      local handle = uv.Tcp.new()
      handle:bind(uv.SockAddrV4.new('0.0.0.0', 0))
      handle:connect(uv.SockAddrV4.new('127.0.0.1', 9123))
      handle:write({MSG})
      handle:startRead()

      local pongCount = 0
      repeat
        local nread, buf = handle:read()
        if nread > 0 then
          if buf:toString(1, nread) ~= MSG then
            error(string.format("got wrong answer %s", buf:toString(1, nread)))
          end
          pongCount = pongCount + 1
          handle:write({MSG})
        end
      until uv.now() - startTime > TIME
      handle:write({'QUIT'})
      handle:close()
      print(string.format("ping_pongs: %d roundtrips/s", pongCount))
    end)
    if err then
      print(err)
    end
  end)()
end

-- We need to use a timer to avoid an error on OSX.
-- test/benchmark-ping-pongs.lua:19: ECONNREFUSED
local timer = uv.Timer.new()
timer:start(timerCb, 10)

uv.run()
