local uv = require 'couv'

local TEST_PORT = 9123

coroutine.wrap(function()
  local handle = uv.Udp.new()
  handle:bind(uv.ip6addr('::0', TEST_PORT))
  handle:startRecv()

  local nread, buf, addr
  repeat
    nread, buf, addr = handle:recv()
    print("udp_server recv nread=", nread)
    if nread and nread > 0 then
      print("udp_server type(nread)=", type(nread))
      print("udp_server recv nread=", nread, ", msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
      handle:send({buf:toString(1, nread)}, addr)
      print("udp_server sent msg=", buf:toString(1, nread))
    end
  until nread and nread == 0

  handle:stopRecv()
  handle:close()
end)()

uv.run()
