local uv = require 'couv'

local exports = {}

exports['interfaceAddresses'] = function(test)
  local addresses = uv.interfaceAddresses()
  test.is_table(addresses)
  for _, address in ipairs(addresses) do
    test.is_string(address.name)
    test.is_boolean(address.isInternal)
    test.ok(uv.SockAddr.isSockAddr(address.address))
    print("name", address.name)
    print("isInternal", address.isInternal)
    print("host", address.address:host())
  end
  test.done()
end

return exports
