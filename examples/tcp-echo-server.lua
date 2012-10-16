local uv = require 'couv'

local co = coroutine.create(function()
  local handle = uv.tcp_create()
  uv.tcp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
  uv.listen(handle, 128, function(server)
    local co2 = coroutine.create(function()
      local stream = uv.tcp_create()
      uv.accept(server, stream)

      uv.read_start(stream)

      local nread, buf
      repeat
        nread, buf = uv.read(stream)
        if nread and nread > 0 then
          uv.write(stream, {buf:toString(1, nread)})
        end
      until nread and nread == 0

      uv.read_stop(stream)

      uv.close(stream)
    end)
    coroutine.resume(co2)
  end)
end)
coroutine.resume(co)

uv.run()
