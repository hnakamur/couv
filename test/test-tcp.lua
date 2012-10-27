local uv = require 'couv'

local exports = {}

exports['tcp.echo'] = function(test)
  coroutine.wrap(function()
    local ok, err = pcall(function()
      local handle = uv.Tcp.new()
      handle:bind(uv.SockAddrV4.new('0.0.0.0', 9123))
      handle:listen(128, function(server)
        coroutine.wrap(function()
          local stream = uv.Tcp.new()
          server:accept(stream)

          stream:startRead()

          local nread, buf
          repeat
            nread, buf = stream:read()
            if nread and nread > 0 then
              stream:write({buf:toString(1, nread)})
            end
          until nread and nread == -1

          stream:stopRead()

          stream:close()
          server:close()
        end)()
      end)
    end)
    if not ok then
      print("err=", err)
    end
  end)()
  
  coroutine.wrap(function()
    local ok, err = pcall(function()
      local handle = uv.Tcp.new()
      handle:connect(uv.SockAddrV4.new('127.0.0.1', 9123))
      handle:startRead()
      handle:write({"PING"})

      local nread, buf = handle:read()
      test.ok(nread)
      test.ok(buf)
      test.equal(nread, #"PING")
      test.equal(buf:toString(1, nread), "PING")

      handle:write({"hello, ", "tcp"})

      nread, buf = handle:read()
      test.ok(nread)
      test.ok(buf)
      test.equal(nread, #"hello, tcp")
      test.equal(buf:toString(1, nread), "hello, tcp")

      handle:stopRead()
      handle:close()
    end)
    if not ok then
      print("err=", err)
    end
  end)()

  uv.run()
  test.done()
end

return exports
