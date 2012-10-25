uv = require 'couv'

function printTable(label, t)
  print(label, t, "{")
  for k, v in pairs(t) do
    print(k, v)
  end
  print("}")
end

coroutine.wrap(function()
  local tcp = uv.tcp_create()
  printTable("getmetatable(tcp)", getmetatable(tcp))
  printTable("getmetatable(getmetatable(tcp))", getmetatable(getmetatable(tcp)))
  printTable("getmetatable(getmetatable(getmetatable(tcp)))", getmetatable(getmetatable(getmetatable(tcp))))
end)()
