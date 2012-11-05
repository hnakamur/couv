local uv = require "couv"

local TEST_PORT = 9123

coroutine.wrap(function()
  local addr = uv.SockAddrV4.new("127.0.0.1", TEST_PORT)
  local tcpServer = uv.Tcp.new()
  tcpServer:bind(addr)
  tcpServer:listen(128, function(server)
    coroutine.wrap(function()
      assert(server == tcpServer)
      handle = uv.Tcp.new()
      server:accept(handle)
      handle:startRead()

      local nread, buf
      repeat
        nread, buf = handle:read()
      until nread and nread < 0

      handle:shutdown()
      handle:close()
      tcpServer:close()
    end)()
  end)
end)()

uv.run()
