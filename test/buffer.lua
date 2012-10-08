local uv = require 'yaluv'
local Buffer = uv.Buffer

local exports = {}

exports['Buffer.new'] = function(test)
  local buf = Buffer.new(2)
  test.equal(type(buf), 'userdata')
  test.done()
end

exports['Buffer.length'] = function(test)
  local buf = Buffer.new(3)
  test.equal(buf:length(), 3)
  test.equal(#buf, 3)
  test.done()
end

exports['Buffer.writeUInt8'] = function(test)
  local buf = Buffer.new(3)
  test.equal(type(buf), 'userdata')
  buf:writeUInt8(1, 97)
  buf:writeUInt8(2, 98)
  buf:writeUInt8(3, 99)
  test.equal(buf[1], 97)
  test.equal(buf[2], 98)
  test.equal(buf[3], 99)
  test.done()
end

exports['Buffer.__newindex'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 97
  buf[2] = 98
  buf[3] = 99
  test.equal(buf:readUInt8(1), 97)
  test.equal(buf:readUInt8(2), 98)
  test.equal(buf:readUInt8(3), 99)
  test.done()
end

exports['Buffer.__index'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 97
  buf[2] = 98
  buf[3] = 99
  test.equal(buf[1], 97)
  test.equal(buf[2], 98)
  test.equal(buf[3], 99)
  test.done()
end

exports['Buffer.slice'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 97
  buf[2] = 98
  buf[3] = 99
  local slice = buf:slice(2, 3)
  test.equal(slice[1], 98)
  test.equal(slice[2], 99)
  slice[1] = 100
  test.equal(slice[1], 100)
  test.equal(buf[2], 100)
  test.done()
end

return exports
