local uv = require 'yaluv'
local loop = uv.loop

local co = coroutine.create(function()
  local handle = uv.tcp_create()
  print("tcp_server#1")
  uv.tcp_bind(handle, uv.ip4addr('0.0.0.0', 9123))
  print("tcp_server#2")
  uv.listen(handle, 128, function(server)
    print("tcp_server#3")
    local co2 = coroutine.create(function()
      print("tcp_server#4")
      local stream = uv.tcp_create()
      print("tcp_server#5")
      uv.accept(server, stream)
      print("tcp_server#6")

      uv.read_start(stream)
      print("tcp_server#7")

      local nread, buf
      repeat
        nread, buf = uv.read(stream)
        print("tcp_server#8, nread=", nread)
        if nread and nread > 0 then
          print("tcp_server#8, msg=", buf:toString(1, nread))
          uv.write(stream, {buf:toString(1, nread)})
          print("tcp_server#9")
        end
      until nread and nread == 0
      print("tcp_server#10")

      uv.read_stop(stream)
      print("tcp_server#11")

      uv.close(stream)
      print("tcp_server#12")
    end)
    coroutine.resume(co2)
  end)
end)
coroutine.resume(co)

loop.get():run()
