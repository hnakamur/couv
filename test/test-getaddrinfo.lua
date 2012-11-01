local uv = require 'couv'
local SockAddr = uv.SockAddr

local exports = {}

exports['getaddrinfo.basic'] = function(test)
  coroutine.wrap(function()
    local name = 'localhost'
    local addresses = uv.getaddrinfo(name, nil, nil)
    test.ok(type(addresses), 'table')
    test.ok(#addresses > 0)
    for i, address in ipairs(addresses) do
      test.ok(type(address), 'table')
      test.ok(address.family == SockAddr.AF_INET or
          address.family == SockAddr.AF_INET6)
      test.ok(address.socktype == SockAddr.SOCK_DGRAM or
          address.socktype == SockAddr.SOCK_STREAM or
          address.socktype == SockAddr.SOCK_DGRAM + SockAddr.SOCK_STREAM)
      test.ok(address.protocol == 0 or
          address.protocol == SockAddr.IPPROTO_TCP or
          address.protocol == SockAddr.IPPROTO_UDP)
      test.ok(SockAddr.isSockAddr(address.addr))
    end
  end)()

  uv.run()
  test.done()
end

exports['getaddrinfo.basic2'] = function(test)
  coroutine.wrap(function()
    local name = 'localhost'
    local hints = {
      family = SockAddr.AF_UNSPEC,
      socktype = SockAddr.SOCK_STREAM,
      protocol = SockAddr.IPPROTO_TCP
    }
    local addresses = uv.getaddrinfo(name, nil, hints)
    test.ok(type(addresses), 'table')
    test.ok(#addresses > 0)
    for i, address in ipairs(addresses) do
      test.ok(type(address), 'table')
      test.ok(address.family == SockAddr.AF_INET or
          address.family == SockAddr.AF_INET6)
      test.ok(address.socktype == SockAddr.SOCK_STREAM)
      test.ok(address.protocol == SockAddr.IPPROTO_TCP)
      test.ok(SockAddr.isSockAddr(address.addr))
    end
  end)()

  uv.run()
  test.done()
end

exports['getaddrinfo.concurrent'] = function(test)
  local CONCURRENT_COUNT = 10
  for i = 1, CONCURRENT_COUNT do
    coroutine.wrap(function()
      local name = 'localhost'
      local addresses = uv.getaddrinfo(name, nil, nil)
      test.ok(type(addresses), 'table')
      test.ok(#addresses > 0)
      for i, address in ipairs(addresses) do
        test.ok(type(address), 'table')
        test.ok(address.family == SockAddr.AF_INET or
            address.family == SockAddr.AF_INET6)
        test.ok(address.socktype == SockAddr.SOCK_DGRAM or
            address.socktype == SockAddr.SOCK_STREAM or
            address.socktype == SockAddr.SOCK_DGRAM + SockAddr.SOCK_STREAM)
        test.ok(address.protocol == 0 or
            address.protocol == SockAddr.IPPROTO_TCP or
            address.protocol == SockAddr.IPPROTO_UDP)
        test.ok(SockAddr.isSockAddr(address.addr))
      end
    end)()
  end

  uv.run()
  test.done()
end

return exports
