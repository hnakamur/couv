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

  buf:writeUInt8(1, 0)
  test.equal(buf[1], 0)

  buf:writeUInt8(1, 255)
  test.equal(buf[1], 255)

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
  buf[1] = 0
  test.equal(buf[1], 0)
  buf[1] = 255
  test.equal(buf[1], 255)
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

exports['Buffer.toString'] = function(test)
  local buf = Buffer.new(3)
  buf[1] = 97
  buf[2] = 98
  buf[3] = 99
  test.equal(buf:toString(1, 1), 'a')
  test.equal(buf:toString(1, 2), 'ab')
  test.equal(buf:toString(2, 2), 'b')
  test.equal(buf:toString(2), 'bc')
  test.equal(buf:toString(3), 'c')
  test.equal(buf:toString(3, 3), 'c')

  local ok, err = pcall(function()
    buf:toString(0)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'toString' (index out of range)", 1, true))

  local ok, err = pcall(buf.toString, buf, 0)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #2 to '?' (index out of range)", 1, true))

  local ok, err = pcall(function()
    buf:toString(1, 4)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #2 to 'toString' (index out of range)", 1, true))

  local ok, err = pcall(function()
    buf:toString(2, 1)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #2 to 'toString' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readUInt16LE'] = function(test)
  local buf = Buffer.new(3)
  buf[1] = 0x12
  buf[2] = 0x34
  buf[3] = 0x56
  test.equal(buf:readUInt16LE(1), 0x3412)
  test.equal(buf:readUInt16LE(2), 0x5634)

  buf[1] = 0xFF
  buf[2] = 0xFF
  test.equal(buf:readUInt16LE(1), 0xFFFF)

  local ok, err = pcall(function()
    buf:readUInt16LE(3)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readUInt16LE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeUInt16LE'] = function(test)
  local buf = Buffer.new(4)
  buf:writeUInt16LE(1, 0x1234)
  test.equal(buf[1], 0x34)
  test.equal(buf[2], 0x12)

  buf:writeUInt16LE(1, 0xFFFF)
  test.equal(buf[1], 0xFF)
  test.equal(buf[2], 0xFF)

  local ok, err = pcall(function()
    buf:writeUInt16LE(4, 0x1234)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeUInt16LE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readUInt16BE'] = function(test)
  local buf = Buffer.new(3)
  buf[1] = 0x12
  buf[2] = 0x34
  buf[3] = 0x56
  test.equal(buf:readUInt16BE(1), 0x1234)
  test.equal(buf:readUInt16BE(2), 0x3456)

  buf[1] = 0xFF
  buf[2] = 0xFF
  test.equal(buf:readUInt16LE(1), 0xFFFF)

  local ok, err = pcall(function()
    buf:readUInt16BE(3)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readUInt16BE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeUInt16BE'] = function(test)
  local buf = Buffer.new(4)
  buf:writeUInt16BE(1, 0x1234)
  test.equal(buf[1], 0x12)
  test.equal(buf[2], 0x34)

  buf:writeUInt16BE(1, 0xFFFF)
  test.equal(buf[1], 0xFF)
  test.equal(buf[2], 0xFF)

  local ok, err = pcall(function()
    buf:writeUInt16BE(4, 0x1234)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeUInt16BE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readUInt32LE'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 0x12
  buf[2] = 0x34
  buf[3] = 0x56
  buf[4] = 0x78
  test.equal(buf:readUInt32LE(1), 0x78563412)

  buf[1] = 0xFF
  buf[2] = 0xFF
  buf[3] = 0xFF
  buf[4] = 0xFF
  test.equal(buf:readUInt32LE(1), 0xFFFFFFFF)

  local ok, err = pcall(function()
    buf:readUInt32LE(2)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readUInt32LE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeUInt32LE'] = function(test)
  local buf = Buffer.new(4)
  buf:writeUInt32LE(1, 0x12345678)
  test.equal(buf[1], 0x78)
  test.equal(buf[2], 0x56)
  test.equal(buf[3], 0x34)
  test.equal(buf[4], 0x12)

  buf:writeUInt32LE(1, 0xFFFFFFFF)
  test.equal(buf[1], 0xFF)
  test.equal(buf[2], 0xFF)
  test.equal(buf[3], 0xFF)
  test.equal(buf[4], 0xFF)

  local ok, err = pcall(function()
    buf:writeUInt32LE(2, 0x12345678)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeUInt32LE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readUInt32BE'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 0x12
  buf[2] = 0x34
  buf[3] = 0x56
  buf[4] = 0x78
  test.equal(buf:readUInt32BE(1), 0x12345678)

  buf[1] = 0xFF
  buf[2] = 0xFF
  buf[3] = 0xFF
  buf[4] = 0xFF
  test.equal(buf:readUInt32BE(1), 0xFFFFFFFF)

  local ok, err = pcall(function()
    buf:readUInt32BE(2)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readUInt32BE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeUInt32BE'] = function(test)
  local buf = Buffer.new(4)
  buf:writeUInt32BE(1, 0x12345678)
  test.equal(buf[1], 0x12)
  test.equal(buf[2], 0x34)
  test.equal(buf[3], 0x56)
  test.equal(buf[4], 0x78)

  buf:writeUInt32BE(1, 0xFFFFFFFF)
  test.equal(buf[1], 0xFF)
  test.equal(buf[2], 0xFF)
  test.equal(buf[3], 0xFF)
  test.equal(buf[4], 0xFF)

  local ok, err = pcall(function()
    buf:writeUInt32BE(2, 0x12345678)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeUInt32BE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readFloatBE'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 0x3F
  buf[2] = 0x80
  buf[3] = 0x00
  buf[4] = 0x00
  test.equal(buf:readFloatBE(1), 1)

  buf[1] = 0xC0
  buf[2] = 0x00
  test.equal(buf:readFloatBE(1), -2)

  buf[1] = 0x3E
  buf[2] = 0x20
  buf[3] = 0x00
  buf[4] = 0x00
  test.equal(buf:readFloatBE(1), 0.15625)

  buf[1] = 0x7F
  buf[2] = 0x7F
  buf[3] = 0xFF
  buf[4] = 0xFF
  local max = 3.4028234663853e+38
  local eps = 1e-14
  test.ok(math.abs(buf:readFloatBE(1) - max) / max < eps)

  local ok, err = pcall(function()
    buf:readFloatBE(2)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readFloatBE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeFloatBE'] = function(test)
  local buf = Buffer.new(4)
  buf:writeFloatBE(1, 1)
  test.equal(buf[1], 0x3F)
  test.equal(buf[2], 0x80)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)

  buf:writeFloatBE(1, -2)
  test.equal(buf[1], 0xC0)
  test.equal(buf[2], 0x00)

  buf:writeFloatBE(1, 0.15625)
  test.equal(buf[1], 0x3E)
  test.equal(buf[2], 0x20)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)

  local max = 3.4028234663853e+38
  buf:writeFloatBE(1, max)
  test.equal(buf[1], 0x7F)
  test.equal(buf[2], 0x7F)
  test.equal(buf[3], 0xFF)
  test.equal(buf[4], 0xFF)

  local ok, err = pcall(function()
    buf:writeFloatBE(2, 1)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeFloatBE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readFloatLE'] = function(test)
  local buf = Buffer.new(4)
  buf[1] = 0x00
  buf[2] = 0x00
  buf[3] = 0x80
  buf[4] = 0x3F
  test.equal(buf:readFloatLE(1), 1)

  buf[3] = 0x00
  buf[4] = 0xC0
  test.equal(buf:readFloatLE(1), -2)

  buf[1] = 0x00
  buf[3] = 0x00
  buf[3] = 0x20
  buf[4] = 0x3E
  test.equal(buf:readFloatLE(1), 0.15625)

  buf[1] = 0xFF
  buf[2] = 0xFF
  buf[3] = 0x7F
  buf[4] = 0x7F
  local max = 3.4028234663853e+38
  local eps = 1e-14
  test.ok(math.abs(buf:readFloatLE(1) - max) / max < eps)

  local ok, err = pcall(function()
    buf:readFloatLE(2)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readFloatLE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeFloatLE'] = function(test)
  local buf = Buffer.new(4)
  buf:writeFloatLE(1, 1)
  test.equal(buf[1], 0x00)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x80)
  test.equal(buf[4], 0x3F)

  buf:writeFloatLE(1, -2)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0xC0)

  buf:writeFloatLE(1, 0.15625)
  test.equal(buf[1], 0x00)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x20)
  test.equal(buf[4], 0x3E)

  local max = 3.4028234663853e+38
  buf:writeFloatLE(1, max)
  test.equal(buf[1], 0xFF)
  test.equal(buf[2], 0xFF)
  test.equal(buf[3], 0x7F)
  test.equal(buf[4], 0x7F)

  local ok, err = pcall(function()
    buf:writeFloatLE(2, 1)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeFloatLE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readDoubleBE'] = function(test)
  local buf = Buffer.new(8)
  buf[1] = 0x3F
  buf[2] = 0xF0
  buf[3] = 0x00
  buf[4] = 0x00
  buf[5] = 0x00
  buf[6] = 0x00
  buf[7] = 0x00
  buf[8] = 0x00
  test.equal(buf:readDoubleBE(1), 1)

  buf[8] = 0x01
  test.equal(buf:readDoubleBE(1), 1.0000000000000002)

  buf[8] = 0x02
  test.equal(buf:readDoubleBE(1), 1.0000000000000004)

  buf[1] = 0x40
  buf[2] = 0x00
  buf[3] = 0x00
  buf[4] = 0x00
  buf[5] = 0x00
  buf[6] = 0x00
  buf[7] = 0x00
  buf[8] = 0x00
  test.equal(buf:readDoubleBE(1), 2)

  buf[1] = 0xC0
  test.equal(buf:readDoubleBE(1), -2)

  buf[1] = 0x7F
  buf[2] = 0xEF
  buf[3] = 0xFF
  buf[4] = 0xFF
  buf[5] = 0xFF
  buf[6] = 0xFF
  buf[7] = 0xFF
  buf[8] = 0xFF
  test.equal(buf:readDoubleBE(1), 1.7976931348623157e+308)

  local ok, err = pcall(function()
    buf:readDoubleBE(2)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readDoubleBE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeDoubleBE'] = function(test)
  local buf = Buffer.new(8)
  buf:writeDoubleBE(1, 1)
  test.equal(buf[1], 0x3F)
  test.equal(buf[2], 0xF0)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0x00)

  buf:writeDoubleBE(1, 1.0000000000000002)
  test.equal(buf[1], 0x3F)
  test.equal(buf[2], 0xF0)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0x01)

  buf:writeDoubleBE(1, 1.0000000000000004)
  test.equal(buf[1], 0x3F)
  test.equal(buf[2], 0xF0)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0x02)

  buf:writeDoubleBE(1, 2)
  test.equal(buf[1], 0x40)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0x00)

  buf:writeDoubleBE(1, -2)
  test.equal(buf[1], 0xC0)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0x00)

  buf:writeDoubleBE(1, 1.7976931348623157e+308)
  test.equal(buf[1], 0x7F)
  test.equal(buf[2], 0xEF)
  test.equal(buf[3], 0xFF)
  test.equal(buf[4], 0xFF)
  test.equal(buf[5], 0xFF)
  test.equal(buf[6], 0xFF)
  test.equal(buf[7], 0xFF)
  test.equal(buf[8], 0xFF)

  local ok, err = pcall(function()
    buf:writeDoubleBE(2, 1)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeDoubleBE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.readDoubleLE'] = function(test)
  local buf = Buffer.new(8)
  buf[1] = 0x00
  buf[2] = 0x00
  buf[3] = 0x00
  buf[4] = 0x00
  buf[5] = 0x00
  buf[6] = 0x00
  buf[7] = 0xF0
  buf[8] = 0x3F
  test.equal(buf:readDoubleLE(1), 1)

  buf[1] = 0x01
  test.equal(buf:readDoubleLE(1), 1.0000000000000002)

  buf[1] = 0x02
  test.equal(buf:readDoubleLE(1), 1.0000000000000004)

  buf[1] = 0x00
  buf[2] = 0x00
  buf[3] = 0x00
  buf[4] = 0x00
  buf[5] = 0x00
  buf[6] = 0x00
  buf[7] = 0x00
  buf[8] = 0x40
  test.equal(buf:readDoubleLE(1), 2)

  buf[8] = 0xC0
  test.equal(buf:readDoubleLE(1), -2)

  buf[1] = 0xFF
  buf[2] = 0xFF
  buf[3] = 0xFF
  buf[4] = 0xFF
  buf[5] = 0xFF
  buf[6] = 0xFF
  buf[7] = 0xEF
  buf[8] = 0x7F
  test.equal(buf:readDoubleLE(1), 1.7976931348623157e+308)

  local ok, err = pcall(function()
    buf:readDoubleLE(2)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'readDoubleLE' (index out of range)", 1, true))

  test.done()
end

exports['Buffer.writeDoubleLE'] = function(test)
  local buf = Buffer.new(8)
  buf:writeDoubleLE(1, 1)
  test.equal(buf[1], 0x00)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0xF0)
  test.equal(buf[8], 0x3F)

  buf:writeDoubleLE(1, 1.0000000000000002)
  test.equal(buf[1], 0x01)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0xF0)
  test.equal(buf[8], 0x3F)

  buf:writeDoubleLE(1, 1.0000000000000004)
  test.equal(buf[1], 0x02)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0xF0)
  test.equal(buf[8], 0x3F)

  buf:writeDoubleLE(1, 2)
  test.equal(buf[1], 0x00)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0x40)

  buf:writeDoubleLE(1, -2)
  test.equal(buf[1], 0x00)
  test.equal(buf[2], 0x00)
  test.equal(buf[3], 0x00)
  test.equal(buf[4], 0x00)
  test.equal(buf[5], 0x00)
  test.equal(buf[6], 0x00)
  test.equal(buf[7], 0x00)
  test.equal(buf[8], 0xC0)

  buf:writeDoubleLE(1, 1.7976931348623157e+308)
  test.equal(buf[1], 0xFF)
  test.equal(buf[2], 0xFF)
  test.equal(buf[3], 0xFF)
  test.equal(buf[4], 0xFF)
  test.equal(buf[5], 0xFF)
  test.equal(buf[6], 0xFF)
  test.equal(buf[7], 0xEF)
  test.equal(buf[8], 0x7F)

  local ok, err = pcall(function()
    buf:writeDoubleLE(2, 1)
  end)
  test.ok(not ok)
  test.ok(string.find(err,
      "bad argument #1 to 'writeDoubleLE' (index out of range)", 1, true))

  test.done()
end

return exports
