local uv = require 'couv'

local exports = {}

exports['ip4addr'] = function(test)
  local addr = uv.ip4addr('127.0.0.1', 80)
  test.equal(addr:host(), '127.0.0.1')
  test.equal(addr:port(), 80)

  test.done()
end

exports['ip6addr'] = function(test)
  local addr = uv.ip6addr('::1', 80)
  test.equal(addr:host(), '::1')
  test.equal(addr:port(), 80)

  test.done()
end

return exports
