local uv = require 'couv'

local exports = {}

exports['sleep'] = function(test)
  coroutine.wrap(function()
    local startTime = uv.now()
    uv.sleep(1000)
    local endTime = uv.now()
    test.ok(endTime - startTime >= 1000)
  end)()

  uv.run()
  test.done()
end

return exports
