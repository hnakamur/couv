local uv = require 'yaluv'
local Buffer = uv.Buffer

local exports = {}

exports['uv.fs_open.sync.OK'] = function(test)
  local err, fd = uv.fs_open('../test/fs.lua', 'r', '0666')
  test.is_nil(err)
  test.is_number(fd)

  err = uv.fs_close(fd)
  test.is_nil(err)

  -- try to close twice
  err = uv.fs_close(fd)
  test.equal(err, 'EBADF')

  test.done()
end

exports['uv.fs_open.sync.ENOENT'] = function(test)
  local err, fd = uv.fs_open('not_exist_file', 'r', '0666')
  test.equal(err, 'ENOENT')
  test.is_nil(fd)
  test.done()
end

exports['uv.fs_open.async.OK'] = function(test)
  local co = coroutine.create(function()
    local err, fd = uv.fs_open('../test/fs.lua', 'r', '0666')
    test.is_nil(err)
    test.is_number(fd)

    err = uv.fs_close(fd)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_open.async.ENOENT'] = function(test)
  local co = coroutine.create(function()
    local err, fd = uv.fs_open('non_exist_file', 'r', '0666')
    test.equal(err, 'ENOENT')
    test.is_nil(fd)
    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_stat.sync'] = function(test)
  local err, stat = uv.fs_stat('Makefile')
  test.is_nil(err)
  test.ok(stat)
  test.ok(stat:isFile())
  test.done()
end

exports['uv.fs_stat.async'] = function(test)
  local co = coroutine.create(function()
    local err, stat = uv.fs_stat('../test/fs.lua')
    test.is_nil(err)
    test.ok(stat:isFile())
    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_write_and_read.sync'] = function(test)
  local path = '_test_uv.fs_write_and_read.sync'
  local err, fd = uv.fs_open(path, 'w', '0666')
  test.is_nil(err)
  local str = 'Hello, libuv!\n'
  local n
  err, n = uv.fs_write(fd, str)
  test.is_nil(err)
  test.equal(n, #str)
  err = uv.fs_close(fd)
  test.is_nil(err)

  err, fd = uv.fs_open(path, 'r')
  test.is_nil(err)
  local buf = Buffer.new(#str)
  err, n = uv.fs_read(fd, buf)
  test.is_nil(err)
  test.equal(buf:toString(), str)
  err = uv.fs_close(fd)
  test.is_nil(err)

  err = uv.fs_unlink(path)
  test.is_nil(err)

  test.done()
end

exports['uv.fs_write_and_read.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_uv.fs_write_and_read.async'
    local err, fd = uv.fs_open(path, 'w', '0666')
    test.is_nil(err)
    local str = 'Hello, libuv!\n'
    local n
    err, n = uv.fs_write(fd, str)
    test.is_nil(err)
    test.equal(n, #str)
    err = uv.fs_close(fd)
    test.is_nil(err)

    err, fd = uv.fs_open(path, 'r')
    test.is_nil(err)
    local buf = Buffer.new(#str)
    err, n = uv.fs_read(fd, buf)
    test.is_nil(err)
    test.equal(buf:toString(), str)
    err = uv.fs_close(fd)
    test.is_nil(err)

    err = uv.fs_unlink(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_unlink.sync'] = function(test)
  local path = '_test_uv.fs_unlink.sync'
  local err, fd = uv.fs_open(path, 'w', '0666')
  test.is_nil(err)
  err = uv.fs_close(fd)
  test.is_nil(err)

  err = uv.fs_unlink(path)
  test.is_nil(err)

  test.done()
end

exports['uv.fs_unlink.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_uv.fs_unlink.async'
    local err, fd = uv.fs_open(path, 'w', '0666')
    test.is_nil(err)
    err = uv.fs_close(fd)
    test.is_nil(err)

    err = uv.fs_unlink(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_mkdir_rmdir.sync'] = function(test)
  local path = '_test_fs_mkdir_rmdir.sync'
  local err = uv.fs_mkdir(path)
  test.is_nil(err)

  local stat
  err, stat = uv.fs_stat(path)
  test.ok(stat:isDirectory())

  err = uv.fs_rmdir(path)
  test.is_nil(err)

  test.done()
end

exports['uv.fs_mkdir_rmdir.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs_mkdir_rmdir.async'
    local err = uv.fs_mkdir(path)
    test.is_nil(err)

    local stat
    err, stat = uv.fs_stat(path)
    test.ok(stat:isDirectory())

    err = uv.fs_rmdir(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_rename_dir.sync'] = function(test)
  local oldPath = '_test_fs_rename_dir.sync.old'
  local newPath = '_test_fs_rename_dir.sync.new'

  test.ok(not uv.fs_exists(oldPath))
  test.ok(not uv.fs_exists(newPath))

  local err = uv.fs_mkdir(oldPath)
  test.is_nil(err)
  test.ok(uv.fs_exists(oldPath))
  test.ok(not uv.fs_exists(newPath))

  err = uv.fs_rename(oldPath, newPath)
  test.is_nil(err)
  test.ok(not uv.fs_exists(oldPath))
  test.ok(uv.fs_exists(newPath))

  err = uv.fs_rmdir(newPath)
  test.is_nil(err)

  test.done()
end

exports['uv.fs_rename_dir.async'] = function(test)
  local co = coroutine.create(function()
    local oldPath = '_test_fs_rename_dir.async.old'
    local newPath = '_test_fs_rename_dir.async.new'

    test.ok(not uv.fs_exists(oldPath))
    test.ok(not uv.fs_exists(newPath))

    local err = uv.fs_mkdir(oldPath)
    test.is_nil(err)
    test.ok(uv.fs_exists(oldPath))
    test.ok(not uv.fs_exists(newPath))

    err = uv.fs_rename(oldPath, newPath)
    test.is_nil(err)
    test.ok(not uv.fs_exists(oldPath))
    test.ok(uv.fs_exists(newPath))

    err = uv.fs_rmdir(newPath)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_ftruncate.sync'] = function(test)
  local path = '_test_fs_ftruncate.sync'
  local err, fd = uv.fs_open(path, 'w', '0666')
  test.is_nil(err)
  local str = 'Hello, libuv!\n'
  local n
  err, n = uv.fs_write(fd, str)
  test.is_nil(err)

  err = uv.fs_ftruncate(fd, 5)
  test.is_nil(err)

  local stat
  err, stat = uv.fs_stat(path)
  test.is_nil(err)
  test.equal(stat:size(), 5)

  err = uv.fs_ftruncate(fd)
  test.is_nil(err)

  err, stat = uv.fs_stat(path)
  test.is_nil(err)
  test.equal(stat:size(), 0)

  err = uv.fs_close(fd)
  test.is_nil(err)

  err = uv.fs_unlink(path)
  test.is_nil(err)

  test.done()
end

exports['uv.fs_ftruncate.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs_ftruncate.async'
    local err, fd = uv.fs_open(path, 'w', '0666')
    test.is_nil(err)
    local str = 'Hello, libuv!\n'
    local n
    err, n = uv.fs_write(fd, str)
    test.is_nil(err)

    err = uv.fs_ftruncate(fd, 5)
    test.is_nil(err)

    local stat
    err, stat = uv.fs_stat(path)
    test.is_nil(err)
    test.equal(stat:size(), 5)

    err = uv.fs_ftruncate(fd)
    test.is_nil(err)

    err, stat = uv.fs_stat(path)
    test.is_nil(err)
    test.equal(stat:size(), 0)

    err = uv.fs_close(fd)
    test.is_nil(err)

    err = uv.fs_unlink(path)
    test.is_nil(err)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

return exports
