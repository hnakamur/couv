local uv = require 'couv'

local PIPENAME = "/tmp/couv-test-sock"

coroutine.wrap(function()
  local handle = uv.Pipe.new()
  if uv.fs.exists(PIPENAME) then
    uv.fs.unlink(PIPENAME)
  end
  handle:bind(PIPENAME)
  handle:listen(128, function(server)
    coroutine.wrap(function()
      local stream = uv.Pipe.new()
      server:accept(stream)

      stream:startRead()

      local nread, buf
      repeat
        nread, buf = stream:read()
        if nread and nread > 0 then
          local msg = buf:toString(1, nread)
          if msg == 'QUIT' then
            handle:close()
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
