local uv = require 'couv'

local co = coroutine.create(function()
  local ok, err = pcall(function()
    local handle = uv.udp_create()

    uv.udp_recv_start(handle)

    uv.udp_send(handle, uv.ip4addr('127.0.0.1', 9123), {"PING", "world"})
    print('udp_client sent {"PING", "world"}')

    local nread, buf, addr = uv.udp_recv(handle)
    print("udp_client recv nread=", nread)
    if nread and nread > 0 then
      print("udp_client recv msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
    end


    uv.udp_send(handle, uv.ip4addr('127.0.0.1', 9123), {"hey"})
    print('udp_client sent {"hey"}')

    nread, buf, addr = uv.udp_recv(handle)
    print("udp_client recv nread=", nread)
    if nread and nread > 0 then
      print("udp_client recv msg=", buf:toString(1, nread), ", host=", addr:host(), ", port=", addr:port())
    end

    uv.udp_recv_stop(handle)
    uv.close(handle)
  end)
  if not ok then
    print("err=", err)
  end
end)
coroutine.resume(co)

uv.run()
