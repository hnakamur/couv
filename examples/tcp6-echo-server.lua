local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local handle = uv.Tcp.new()
  handle:bind(uv.SockAddrV6.new('::1', TEST_PORT))
  handle:listen(128, function(server)
    coroutine.wrap(function()
      local stream = uv.Tcp.new()
      server:accept(stream)

      stream:startRead()

      local nread, buf
      repeat
        nread, buf = stream:read()
        if nread and nread > 0 then
          local msg = buf:toString(1, nread)
          if msg == 'QUIT' then
            server:close()
            break
          end
          stream:write({msg})
        end
      until nread and nread == 0

      stream:stopRead()

      stream:close()
    end)()
  end)
end)()

uv.run()
