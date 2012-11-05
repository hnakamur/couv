local uv = require 'couv'

local exports = {}

exports['cpuInfo'] = function(test)
  local cpuInfos = uv.cpuInfo()
  test.is_table(cpuInfos)
  for _, cpuInfo in ipairs(cpuInfos) do
    test.is_string(cpuInfo.model)
    test.is_number(cpuInfo.speed)
    test.is_number(cpuInfo.user)
    test.is_number(cpuInfo.nice)
    test.is_number(cpuInfo.sys)
    test.is_number(cpuInfo.idle)
    test.is_number(cpuInfo.irq)
    print("model", cpuInfo.model)
    print("speed", cpuInfo.speed)
    print("user", cpuInfo.user)
    print("nice", cpuInfo.nice)
    print("sys", cpuInfo.sys)
    print("idle", cpuInfo.idle)
    print("irq", cpuInfo.irq)
  end
  test.done()
end

return exports
