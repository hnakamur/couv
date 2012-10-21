local uv = require 'couv'

local exports = {}

local TEST_PORT = 9123

exports['tcp.sockname'] = function(test)
  local connectPort
  coroutine.wrap(function()
    local handle = uv.tcp_create()
    uv.tcp_bind(handle, uv.ip4addr('127.0.0.1', TEST_PORT))
    uv.listen(handle, 128, function(server)
      coroutine.wrap(function()
        local stream = uv.tcp_create()
        uv.accept(server, stream)


        local sockname = uv.tcp_getsockname(stream)
        test.equal(sockname:host(), '127.0.0.1')
        test.equal(sockname:port(), TEST_PORT)

        local ok, err = pcall(uv.tcp_getpeername, stream)
        local peername = uv.tcp_getpeername(stream)
        test.equal(peername:host(), '127.0.0.1')
        test.equal(peername:port(), connectPort)

        uv.read_start(stream)
        local nread, buf
        repeat
          nread, buf = uv.read(stream)
        until nread and nread <= 0
        uv.read_stop(stream)
        uv.close(stream)

        uv.close(server)
      end)()
    end)

    local sockname = uv.tcp_getsockname(handle)
    test.equal(sockname:host(), '127.0.0.1')
    test.equal(sockname:port(), TEST_PORT)

    local ok, err = pcall(uv.tcp_getpeername, handle)
    test.ok(not ok)
    test.equal(string.sub(err, -#'ENOTCONN'), 'ENOTCONN')
  end)()
  
  coroutine.wrap(function()
    local handle = uv.tcp_create()
    uv.tcp_connect(handle, uv.ip4addr('127.0.0.1', TEST_PORT))
    local sockname = uv.tcp_getsockname(handle)
    test.ok(sockname)
    test.ok(sockname:port() > 0)
    connectPort = sockname:port()

    local peername = uv.tcp_getpeername(handle)
    test.equal(peername:host(), '127.0.0.1')
    test.equal(peername:port(), TEST_PORT)

    uv.close(handle)
  end)()

  uv.run()
  test.done()
end

exports['udp.sockname'] = function(test)
  -- listener
  coroutine.wrap(function()
    local handle = uv.udp_create()
    uv.udp_bind(handle, uv.ip4addr('0.0.0.0', TEST_PORT))

    local sockname = uv.udp_getsockname(handle)
    test.equal(sockname:host(), '0.0.0.0')
    test.equal(sockname:port(), TEST_PORT)

    uv.udp_recv_start(handle)

    uv.close(handle)
  end)()

  -- sender
  coroutine.wrap(function()
    local handle = uv.udp_create()
    local serverAddr = uv.ip4addr('127.0.0.1', TEST_PORT)

    uv.udp_send(handle, {"PING"}, serverAddr)

    local sockname = uv.udp_getsockname(handle)
    test.equal(sockname:host(), '0.0.0.0')
    test.ok(sockname:port() > 0)

    uv.close(handle)
  end)()

  uv.run()
  test.done()
end

return exports
