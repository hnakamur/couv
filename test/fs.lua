local uv = require 'yaluv'
local fs = uv.fs
local loop = uv.loop.default()

local exports = {}

exports['fs.open.sync.OK'] = function(test)
  local err, fd = fs.open(loop, '../test/fs.lua', 'r', '0666')
  test.is_nil(err)
  test.is_number(fd)

  err = fs.close(loop, fd)
  test.is_nil(err)

  test.done()
end

exports['fs.open.sync.ENOENT'] = function(test)
  local err, fd = fs.open(loop, 'not_exist_file', 'r', '0666')
  test.equal(err, 'ENOENT')
  test.is_nil(fd)
  test.done()
end

exports['fs.open.async.OK'] = function(test)
  coroutine.wrap(function()
    local err, fd = fs.open(loop, '../test/fs.lua', 'r', '0666')
    test.is_nil(err)
    test.is_number(fd)

    err = fs.close(loop, fd)
    test.is_nil(err)

    test.done()
  end)()

  loop:run()
end

exports['fs.open.async.ENOENT'] = function(test)
  coroutine.wrap(function()
    local err, fd = fs.open(loop, 'non_exist_file', 'r', '0666')
    test.equal(err, 'ENOENT')
    test.is_nil(fd)
    test.done()
  end)()

  loop:run()
end

exports['fs.stat.sync'] = function(test)
  local err, stat = fs.stat(loop, 'Makefile')
  test.is_nil(err)
  test.ok(stat)
  test.ok(stat:isFile())
  test.done()
end

exports['fs.stat.async'] = function(test)
  coroutine.wrap(function()
    local err, stat = fs.stat(loop, '../test/fs.lua')
    test.is_nil(err)
    test.ok(stat:isFile())
    test.done()
  end)()

  loop:run()
end

return exports
