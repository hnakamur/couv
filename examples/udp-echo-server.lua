local uv = require 'couv'

local co = coroutine.create(function()
  local ok, err = pcall(function()
    local handle = uv.udp_create()
    uv.udp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
    uv.udp_recv_start(handle)

    local nread, buf, addr
    repeat
      nread, buf, addr = uv.udp_recv(handle)
      print("udp_server recv nread=", nread)
      if nread and nread > 0 then
        print("udp_server type(nread)=", type(nread))
        print("udp_server recv nread=", nread, ", msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
        uv.udp_send(handle, addr, {buf:toString(1, nread)})
        print("udp_server sent msg=", buf:toString(1, nread))
      end
    until nread and nread == 0

    uv.udp_recv_stop(handle)
    uv.close(handle)
  end)
  if not ok then
    print("err=", err)
  end
end)
coroutine.resume(co)

uv.run()
