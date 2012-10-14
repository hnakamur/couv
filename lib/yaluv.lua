local uv = require 'yaluv_native'

uv.udp_recv = function(handle)
  local nread, buf, addr
  repeat
    nread, buf, addr = uv.udp_prim_recv(handle)
  until nread
  return nread, buf, addr
end

return uv
