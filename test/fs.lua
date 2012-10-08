local uv = require 'yaluv'
local fs = uv.fs
local loop = uv.loop
local Buffer = uv.Buffer

local exports = {}

exports['fs.open.sync.OK'] = function(test)
  local err, fd = fs.open('../test/fs.lua', 'r', '0666')
  test.is_nil(err)
  test.is_number(fd)

  err = fs.close(fd)
  test.is_nil(err)

  -- try to close twice
  err = fs.close(fd)
  test.equal(err, 'EBADF')

  test.done()
end

exports['fs.open.sync.ENOENT'] = function(test)
  local err, fd = fs.open('not_exist_file', 'r', '0666')
  test.equal(err, 'ENOENT')
  test.is_nil(fd)
  test.done()
end

exports['fs.open.async.OK'] = function(test)
  coroutine.wrap(function()
    local err, fd = fs.open('../test/fs.lua', 'r', '0666')
    test.is_nil(err)
    test.is_number(fd)

    err = fs.close(fd)
    test.is_nil(err)

    test.done()
  end)()

  loop.get():run()
end

exports['fs.open.async.ENOENT'] = function(test)
  coroutine.wrap(function()
    local err, fd = fs.open('non_exist_file', 'r', '0666')
    test.equal(err, 'ENOENT')
    test.is_nil(fd)
    test.done()
  end)()

  loop.get():run()
end

exports['fs.stat.sync'] = function(test)
  local err, stat = fs.stat('Makefile')
  test.is_nil(err)
  test.ok(stat)
  test.ok(stat:isFile())
  test.done()
end

exports['fs.stat.async'] = function(test)
  coroutine.wrap(function()
    local err, stat = fs.stat('../test/fs.lua')
    test.is_nil(err)
    test.ok(stat:isFile())
    test.done()
  end)()

  loop.get():run()
end

exports['fs.write_and_read.sync'] = function(test)
  local path = '_test_fs.write_and_read.sync'
  local err, fd = fs.open(path, 'w', '0666')
  test.is_nil(err)
  local str = 'Hello, libuv!\n'
  local n
  err, n = fs.write(fd, str)
  test.is_nil(err)
  test.equal(n, #str)
  err = fs.close(fd)
  test.is_nil(err)

  err, fd = fs.open(path, 'r')
  test.is_nil(err)
  local buf = Buffer.new(#str)
  err, n = fs.read(fd, buf)
  test.is_nil(err)
  test.equal(buf:toString(), str)
  err = fs.close(fd)
  test.is_nil(err)

  test.done()
end

exports['fs.write_and_read.async'] = function(test)
  coroutine.wrap(function()
    local path = '_test_fs.write_and_read.async'
    local err, fd = fs.open(path, 'w', '0666')
    test.is_nil(err)
    local str = 'Hello, libuv!\n'
    local n
    err, n = fs.write(fd, str)
    test.is_nil(err)
    test.equal(n, #str)
    err = fs.close(fd)
    test.is_nil(err)

    err, fd = fs.open(path, 'r')
    test.is_nil(err)
    local buf = Buffer.new(#str)
    err, n = fs.read(fd, buf)
    test.is_nil(err)
    test.equal(buf:toString(), str)
    err = fs.close(fd)
    test.is_nil(err)

    test.done()
  end)()

  loop.get():run()
end

return exports
