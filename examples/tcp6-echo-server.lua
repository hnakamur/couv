local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local handle = uv.tcp_create()
  uv.tcp_bind(handle, uv.ip6addr('::1', TEST_PORT))
  uv.listen(handle, 128, function(server)
    coroutine.wrap(function()
      local stream = uv.tcp_create()
      uv.accept(server, stream)

      uv.read_start(stream)

      local nread, buf
      repeat
        nread, buf = uv.read(stream)
        if nread and nread > 0 then
          local msg = buf:toString(1, nread)
          if msg == 'QUIT' then
            uv.close(server)
            break
          end
          uv.write(stream, {msg})
        end
      until nread and nread == 0

      uv.read_stop(stream)

      uv.close(stream)
    end)()
  end)
end)()

uv.run()
