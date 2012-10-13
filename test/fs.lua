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
  local co = coroutine.create(function()
    local err, fd = fs.open('../test/fs.lua', 'r', '0666')
    test.is_nil(err)
    test.is_number(fd)

    err = fs.close(fd)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  loop.get():run()
end

exports['fs.open.async.ENOENT'] = function(test)
  local co = coroutine.create(function()
    local err, fd = fs.open('non_exist_file', 'r', '0666')
    test.equal(err, 'ENOENT')
    test.is_nil(fd)
    test.done()
  end)
  coroutine.resume(co)

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
  local co = coroutine.create(function()
    local err, stat = fs.stat('../test/fs.lua')
    test.is_nil(err)
    test.ok(stat:isFile())
    test.done()
  end)
  coroutine.resume(co)

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

  err = fs.unlink(path)
  test.is_nil(err)

  test.done()
end

exports['fs.write_and_read.async'] = function(test)
  local co = coroutine.create(function()
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

    err = fs.unlink(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  loop.get():run()
end

exports['fs.unlink.sync'] = function(test)
  local path = '_test_fs.unlink.sync'
  local err, fd = fs.open(path, 'w', '0666')
  test.is_nil(err)
  err = fs.close(fd)
  test.is_nil(err)

  err = fs.unlink(path)
  test.is_nil(err)

  test.done()
end

exports['fs.unlink.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs.unlink.async'
    local err, fd = fs.open(path, 'w', '0666')
    test.is_nil(err)
    err = fs.close(fd)
    test.is_nil(err)

    err = fs.unlink(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  loop.get():run()
end

exports['fs.mkdir_rmdir.sync'] = function(test)
  local path = '_test_fs_mkdir_rmdir.sync'
  local err = fs.mkdir(path)
  test.is_nil(err)

  local stat
  err, stat = fs.stat(path)
  test.ok(stat:isDirectory())

  err = fs.rmdir(path)
  test.is_nil(err)

  test.done()
end

exports['fs.mkdir_rmdir.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs_mkdir_rmdir.async'
    local err = fs.mkdir(path)
    test.is_nil(err)

    local stat
    err, stat = fs.stat(path)
    test.ok(stat:isDirectory())

    err = fs.rmdir(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  loop.get():run()
end

exports['fs.rename_dir.sync'] = function(test)
  local oldPath = '_test_fs_rename_dir.sync.old'
  local newPath = '_test_fs_rename_dir.sync.new'

  test.ok(not fs.exists(oldPath))
  test.ok(not fs.exists(newPath))

  local err = fs.mkdir(oldPath)
  test.is_nil(err)
  test.ok(fs.exists(oldPath))
  test.ok(not fs.exists(newPath))

  err = fs.rename(oldPath, newPath)
  test.is_nil(err)
  test.ok(not fs.exists(oldPath))
  test.ok(fs.exists(newPath))

  err = fs.rmdir(newPath)
  test.is_nil(err)

  test.done()
end

--[[ TODO: investigate why this test blocks.
exports['fs.rename_dir.async'] = function(test)
  local co = coroutine.create(function()
    local oldPath = '_test_fs_rename_dir.async.old'
    local newPath = '_test_fs_rename_dir.async.new'

    test.ok(not fs.exists(oldPath))
    test.ok(not fs.exists(newPath))

    local err = fs.mkdir(oldPath)
    test.is_nil(err)
    test.ok(fs.exists(oldPath))
    test.ok(not fs.exists(newPath))

    err = fs.rename(oldPath, newPath)
    test.is_nil(err)
    test.ok(not fs.exists(oldPath))
    test.ok(fs.exists(newPath))

    err = fs.rmdir(newPath)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  loop.get():run()
end
]]

exports['fs.ftruncate.sync'] = function(test)
  local path = '_test_fs_ftruncate.sync'
  local err, fd = fs.open(path, 'w', '0666')
  test.is_nil(err)
  local str = 'Hello, libuv!\n'
  local n
  err, n = fs.write(fd, str)
  test.is_nil(err)

  err = fs.ftruncate(fd, 5)
  test.is_nil(err)

  local stat
  err, stat = fs.stat(path)
  test.is_nil(err)
  test.equal(stat:size(), 5)

  err = fs.ftruncate(fd)
  test.is_nil(err)

  err, stat = fs.stat(path)
  test.is_nil(err)
  test.equal(stat:size(), 0)

  err = fs.close(fd)
  test.is_nil(err)

  err = fs.unlink(path)
  test.is_nil(err)

  test.done()
end

exports['fs.ftruncate.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs_ftruncate.async'
    local err, fd = fs.open(path, 'w', '0666')
    test.is_nil(err)
    local str = 'Hello, libuv!\n'
    local n
    err, n = fs.write(fd, str)
    test.is_nil(err)

    err = fs.ftruncate(fd, 5)
    test.is_nil(err)

    local stat
    err, stat = fs.stat(path)
    test.is_nil(err)
    test.equal(stat:size(), 5)

    err = fs.ftruncate(fd)
    test.is_nil(err)

    err, stat = fs.stat(path)
    test.is_nil(err)
    test.equal(stat:size(), 0)

    err = fs.close(fd)
    test.is_nil(err)

    err = fs.unlink(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  loop.get():run()
end

return exports
