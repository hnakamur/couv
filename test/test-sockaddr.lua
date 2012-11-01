local uv = require 'couv'

local exports = {}

exports['SockAddrV4'] = function(test)
  local addr = uv.SockAddrV4.new('127.0.0.1', 80)
  test.equal(addr:host(), '127.0.0.1')
  test.equal(addr:port(), 80)
  test.ok(addr:isV4())

  test.done()
end

exports['SockAddrV6'] = function(test)
  local addr = uv.SockAddrV6.new('::1', 80)
  test.equal(addr:host(), '::1')
  test.equal(addr:port(), 80)
  test.ok(addr:isV6())

  test.done()
end

exports['SockAddr'] = function(test)
  local addr = uv.SockAddr.create('127.0.0.1', 80)
  test.equal(addr:host(), '127.0.0.1')
  test.equal(addr:port(), 80)
  test.ok(addr:isV4())

  addr = uv.SockAddr.create('127.0.0.1', 80, uv.SockAddr.AF_INET)
  test.equal(addr:host(), '127.0.0.1')
  test.equal(addr:port(), 80)
  test.ok(addr:isV4())

  local ok, err = pcall(uv.SockAddr.create, '127.0.0.1', 80,
      uv.SockAddr.AF_INET6)
  test.ok(not ok)
  test.ok(string.find(err, 'must be valid IP address', 1, false))

  addr = uv.SockAddr.create('::1', 80)
  test.equal(addr:host(), '::1')
  test.equal(addr:port(), 80)
  test.ok(addr:isV6())

  addr = uv.SockAddr.create('::1', 80, uv.SockAddr.AF_INET6)
  test.equal(addr:host(), '::1')
  test.equal(addr:port(), 80)
  test.ok(addr:isV6())

  test.done()
end

return exports
