local uv = require 'couv'

local PIPENAME = "/tmp/couv-test-sock"

coroutine.wrap(function()
  local handle = uv.pipe_create()
  if uv.fs_exists(PIPENAME) then
    uv.fs_unlink(PIPENAME)
  end
  uv.pipe_bind(handle, PIPENAME)
  uv.listen(handle, 128, function(server)
    coroutine.wrap(function()
      local stream = uv.pipe_create()
      uv.accept(server, stream)

      uv.read_start(stream)

      local nread, buf
      repeat
        nread, buf = uv.read(stream)
        if nread and nread > 0 then
          local msg = buf:toString(1, nread)
          if msg == 'QUIT' then
            uv.close(handle)
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
