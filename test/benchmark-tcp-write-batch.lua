local uv = require 'couv'

local WRITE_REQ_DATA = "Hello, world."
local NUM_WRITE_REQS = 1000 * 1000
local TEST_PORT = 9123

function exitCb(process, exitStatus, termSignal)
  process:close()
end
local process = uv.Process.spawn{
    args={uv.exepath(), 'test/tcp4-blackhole-server.lua'},
    flags=uv.Process.DETACHED,
    exitCb=exitCb}

coroutine.wrap(function()
  local ok, err = pcall(function()
    -- We need sleep to avoid an ECONNREFUSED error on OSX.
    uv.sleep(10)

    local tcpClient = uv.Tcp.new()
    tcpClient:connect(uv.SockAddrV4.new('127.0.0.1', 9123))

    -- NOTE: This is different from benchmark-tcp-write-batch in libuv.
    -- We are forced to wait until write callback is called.
    for i = 1, NUM_WRITE_REQS do
      tcpClient:write({WRITE_REQ_DATA})
    end
    tcpClient:shutdown()
    tcpClient:close()
  end)
  if err then
    print(err)
  end
end)()

local start = uv.hrtime()
uv.run()
local stop = uv.hrtime()
print(string.format("%d write requests in %.2fs", NUM_WRITE_REQS,
    (stop - start) / 1e9))
