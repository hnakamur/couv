local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['tcp.sockname'] = function(test)
  local connectPort
  coroutine.wrap(function()
    local handle = uv.Tcp.new()
    handle:bind(uv.SockAddrV4.new('127.0.0.1', TEST_PORT))
    handle:listen(128, function(server)
      coroutine.wrap(function()
        local stream = uv.Tcp.new()
        server:accept(stream)


        local sockname = stream:getsockname()
        test.equal(sockname:host(), '127.0.0.1')
        test.equal(sockname:port(), TEST_PORT)

        local ok, err = pcall(stream.getpeername, stream)
        local peername = stream:getpeername()
        test.equal(peername:host(), '127.0.0.1')
        test.equal(peername:port(), connectPort)

        stream:startRead()
        local nread, buf
        repeat
          nread, buf = stream:read()
        until nread and nread <= 0
        stream:stopRead()
        stream:close()

        server:close()
      end)()
    end)

    local sockname = handle:getsockname()
    test.equal(sockname:host(), '127.0.0.1')
    test.equal(sockname:port(), TEST_PORT)

    local ok, err = pcall(handle.getpeername, handle)
    test.ok(not ok)
    test.equal(string.sub(err, -#'ENOTCONN'), 'ENOTCONN')
  end)()
  
  coroutine.wrap(function()
    local handle = uv.Tcp.new()
    handle:connect(uv.SockAddrV4.new('127.0.0.1', TEST_PORT))
    local sockname = handle:getsockname()
    test.ok(sockname)
    test.ok(sockname:port() > 0)
    connectPort = sockname:port()

    local peername = handle:getpeername()
    test.equal(peername:host(), '127.0.0.1')
    test.equal(peername:port(), TEST_PORT)

    handle:close()
  end)()

  uv.run()
  test.done()
end

exports['udp.sockname'] = function(test)
  -- listener
  coroutine.wrap(function()
    local handle = uv.Udp.new()
    handle:bind(uv.SockAddrV4.new('0.0.0.0', TEST_PORT))

    local sockname = handle:getsockname()
    test.equal(sockname:host(), '0.0.0.0')
    test.equal(sockname:port(), TEST_PORT)

    handle:startRecv()

    handle:close()
  end)()

  -- sender
  coroutine.wrap(function()
    local handle = uv.Udp.new()
    local serverAddr = uv.SockAddrV4.new('127.0.0.1', TEST_PORT)

    handle:send({"PING"}, serverAddr)

    local sockname = handle:getsockname()
    test.equal(sockname:host(), '0.0.0.0')
    test.ok(sockname:port() > 0)

    handle:close()
  end)()

  uv.run()
  test.done()
end

return exports
