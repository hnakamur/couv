local uv = require 'couv'

local exports = {}

exports['hrtime'] = function(test)
  local t = uv.hrtime()
  test.is_number(t)
  test.done()
end

return exports
