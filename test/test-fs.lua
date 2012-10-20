local uv = require 'couv'
local Buffer = uv.Buffer

local exports = {}

exports['uv.fs_open.sync.OK'] = function(test)
  local fd = uv.fs_open('couv.lua', 'r', '0666')
  test.is_number(fd)

  uv.fs_close(fd)

  -- try to close twice
  local ok, err = pcall(uv.fs_close, fd)
  test.ok(not ok)
  test.equal(string.sub(err, -string.len('EBADF'), -1), 'EBADF')

  test.done()
end

exports['uv.fs_open.sync.ENOENT'] = function(test)
  local ok, err = pcall(function()
    uv.fs_open('not_exist_file', 'r', '0666')
  end)
  test.ok(not ok)
  test.equal(string.sub(err, -string.len('ENOENT'), -1), 'ENOENT')
  test.done()
end

exports['uv.fs_open.async.OK'] = function(test)
  local co = coroutine.create(function()
    local fd = uv.fs_open('couv.lua', 'r', '0666')
    test.is_number(fd)

    uv.fs_close(fd)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_open.async.ENOENT'] = function(test)
  local co = coroutine.create(function()
    local ok, err = pcall(function()
      uv.fs_open('not_exist_file', 'r', '0666')
    end)
    test.ok(not ok)
    test.equal(string.sub(err, -string.len('ENOENT'), -1), 'ENOENT')
    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_stat.sync'] = function(test)
  local stat = uv.fs_stat('Makefile')
  test.ok(stat)
  test.ok(stat:isFile())
  test.done()
end

exports['uv.fs_stat.async'] = function(test)
  coroutine.wrap(function()
    local stat = uv.fs_stat('couv.lua')
    test.ok(stat:isFile())
    test.done()
  end)()

  uv.run()
end

exports['uv.fs_write_and_read.sync'] = function(test)
  local path = '_test_uv.fs_write_and_read.sync'
  local fd = uv.fs_open(path, 'w', '0666')
  local str = 'Hello, libuv!\n'
  local n = uv.fs_write(fd, str)
  test.equal(n, #str)
  uv.fs_close(fd)

  fd = uv.fs_open(path, 'r')
  local buf = Buffer.new(#str)
  n = uv.fs_read(fd, buf)
  test.equal(buf:toString(), str)
  uv.fs_close(fd)

  uv.fs_unlink(path)

  test.done()
end

exports['uv.fs_write_and_read.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_uv.fs_write_and_read.async'
    local fd = uv.fs_open(path, 'w', '0666')
    local str = 'Hello, libuv!\n'
    local n = uv.fs_write(fd, str)
    test.equal(n, #str)
    uv.fs_close(fd)

    fd = uv.fs_open(path, 'r')
    local buf = Buffer.new(#str)
    n = uv.fs_read(fd, buf)
    test.equal(buf:toString(), str)
    uv.fs_close(fd)

    uv.fs_unlink(path)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_unlink.sync'] = function(test)
  local path = '_test_uv.fs_unlink.sync'
  local fd = uv.fs_open(path, 'w', '0666')
  uv.fs_close(fd)

  uv.fs_unlink(path)

  test.ok(true)
  test.done()
end

exports['uv.fs_unlink.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_uv.fs_unlink.async'
    local fd = uv.fs_open(path, 'w', '0666')
    uv.fs_close(fd)

    uv.fs_unlink(path)

    test.ok(true)
    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_mkdir_rmdir.sync'] = function(test)
  local path = '_test_fs_mkdir_rmdir.sync'
  uv.fs_mkdir(path)

  local stat = uv.fs_stat(path)
  test.ok(stat:isDirectory())

  uv.fs_rmdir(path)

  test.done()
end

exports['uv.fs_mkdir_rmdir.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs_mkdir_rmdir.async'
    uv.fs_mkdir(path)

    local stat = uv.fs_stat(path)
    test.ok(stat:isDirectory())

    uv.fs_rmdir(path)

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

  uv.fs_mkdir(oldPath)
  test.ok(uv.fs_exists(oldPath))
  test.ok(not uv.fs_exists(newPath))

  uv.fs_rename(oldPath, newPath)
  test.ok(not uv.fs_exists(oldPath))
  test.ok(uv.fs_exists(newPath))

  uv.fs_rmdir(newPath)

  test.done()
end

exports['uv.fs_rename_dir.async'] = function(test)
  local co = coroutine.create(function()
    local oldPath = '_test_fs_rename_dir.async.old'
    local newPath = '_test_fs_rename_dir.async.new'

    test.ok(not uv.fs_exists(oldPath))
    test.ok(not uv.fs_exists(newPath))

    uv.fs_mkdir(oldPath)
    test.ok(uv.fs_exists(oldPath))
    test.ok(not uv.fs_exists(newPath))

    uv.fs_rename(oldPath, newPath)
    test.ok(not uv.fs_exists(oldPath))
    test.ok(uv.fs_exists(newPath))

    uv.fs_rmdir(newPath)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

exports['uv.fs_ftruncate.sync'] = function(test)
  local path = '_test_fs_ftruncate.sync'
  local fd = uv.fs_open(path, 'w', '0666')
  local str = 'Hello, libuv!\n'
  local n = uv.fs_write(fd, str)

  uv.fs_ftruncate(fd, 5)

  local stat = uv.fs_stat(path)
  test.equal(stat:size(), 5)

  uv.fs_ftruncate(fd)

  stat = uv.fs_stat(path)
  test.equal(stat:size(), 0)

  uv.fs_close(fd)

  uv.fs_unlink(path)

  test.done()
end

exports['uv.fs_ftruncate.async'] = function(test)
  local co = coroutine.create(function()
    local path = '_test_fs_ftruncate.async'
    local fd = uv.fs_open(path, 'w', '0666')
    local str = 'Hello, libuv!\n'
    local n = uv.fs_write(fd, str)

    uv.fs_ftruncate(fd, 5)

    local stat = uv.fs_stat(path)
    test.equal(stat:size(), 5)

    uv.fs_ftruncate(fd)

    stat = uv.fs_stat(path)
    test.equal(stat:size(), 0)

    uv.fs_close(fd)

    uv.fs_unlink(path)

    test.done()
  end)
  coroutine.resume(co)

  uv.run()
end

return exports
