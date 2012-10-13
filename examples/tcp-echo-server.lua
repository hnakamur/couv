local uv = require 'yaluv'
local loop = uv.loop

local co = coroutine.create(function()
  local handle = uv.tcp_create()
  uv.tcp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
  uv.listen(handle, 128, function(server)
    local co2 = coroutine.create(function()
      local stream = uv.tcp_create()
      uv.accept(server, stream)
      local nread, buf = uv.read_start(stream)
      if nread == 0 then
        uv.close(stream)
        return
      end
      print("tcp server nread=", nread, ", received=", buf:toString(1, nread))
      uv.write(stream, {buf:slice(1, nread)})
    end)
    coroutine.resume(co2)
  end)
end)
coroutine.resume(co)

loop.get():run()
