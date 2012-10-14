local uv = require 'yaluv_native'

uv.udp_recv = function(handle)
  local nread, buf, addr
  repeat
    nread, buf, addr = uv.udp_prim_recv(handle)
  until nread
  return nread, buf, addr
end

uv.read = function(handle)
  local nread, buf
  repeat
    nread, buf, addr = uv.prim_read(handle)
  until nread
  return nread, buf
end

return uv
