local uv = require 'couv'

local exports = {}

exports['tcp.echo'] = function(test)
  local server = coroutine.create(function()
    local ok, err = pcall(function()
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
          until nread and nread == -1

          uv.read_stop(stream)

          uv.close(stream)
          uv.close(server)
        end)
        coroutine.resume(co2)
      end)
    end)
    if not ok then
      print("err=", err)
    end
  end)
  coroutine.resume(server)
  
  local client = coroutine.create(function()
    local ok, err = pcall(function()
      local handle = uv.tcp_create()
      uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', 9123))
      uv.read_start(handle)
      uv.write(handle, {"PING"})

      local nread, buf = uv.read(handle)
      test.ok(nread)
      test.ok(buf)
      test.equal(nread, #"PING")
      test.equal(buf:toString(1, nread), "PING")

      uv.write(handle, {"hello, ", "tcp"})

      nread, buf = uv.read(handle)
      test.ok(nread)
      test.ok(buf)
      test.equal(nread, #"hello, tcp")
      test.equal(buf:toString(1, nread), "hello, tcp")

      uv.read_stop(handle)
      uv.close(handle)
    end)
    if not ok then
      print("err=", err)
    end
  end)
  coroutine.resume(client)

  uv.run()
  test.done()
end

return exports
