local uv = require 'couv'
local Buffer = uv.Buffer

local exports = {}

exports['uv.fs.open.sync.OK'] = function(test)
  local fd = uv.fs.open('couv.lua', 'r', '0666')
  test.is_number(fd)

  uv.fs.close(fd)

  -- try to close twice
  local ok, err = pcall(uv.fs.close, fd)
  test.ok(not ok)
  test.equal(string.sub(err, -string.len('EBADF'), -1), 'EBADF')

  test.done()
end

exports['uv.fs.open.sync.ENOENT'] = function(test)
  local ok, err = pcall(function()
    uv.fs.open('not_exist_file', 'r', '0666')
  end)
  test.ok(not ok)
  test.equal(string.sub(err, -string.len('ENOENT'), -1), 'ENOENT')
  test.done()
end

exports['uv.fs.open.async.OK'] = function(test)
  coroutine.wrap(function()
    local fd = uv.fs.open('couv.lua', 'r', '0666')
    test.is_number(fd)

    uv.fs.close(fd)

    test.done()
  end)()

  uv.run()
end

exports['uv.fs.open.async.ENOENT'] = function(test)
  coroutine.wrap(function()
    local ok, err = pcall(function()
      uv.fs.open('not_exist_file', 'r', '0666')
    end)
    test.ok(not ok)
    test.equal(string.sub(err, -string.len('ENOENT'), -1), 'ENOENT')
    test.done()
  end)()

  uv.run()
end

exports['uv.fs.stat.sync'] = function(test)
  local stat = uv.fs.stat('Makefile')
  test.ok(stat)
  test.ok(stat:isFile())
  test.done()
end

exports['uv.fs.stat.async'] = function(test)
  coroutine.wrap(function()
    local stat = uv.fs.stat('couv.lua')
    test.ok(stat:isFile())
    test.done()
  end)()

  uv.run()
end

exports['uv.fs.write_and_read.sync'] = function(test)
  local path = '_test_uv.fs.write_and_read.sync'
  local fd = uv.fs.open(path, 'w', '0666')
  local str = 'Hello, libuv!\n'
  local n = uv.fs.write(fd, str)
  test.equal(n, #str)
  uv.fs.close(fd)

  fd = uv.fs.open(path, 'r')
  local buf = Buffer.new(#str)
  n = uv.fs.read(fd, buf)
  test.equal(buf:toString(), str)
  uv.fs.close(fd)

  uv.fs.unlink(path)

  test.done()
end

exports['uv.fs.write_and_read.async'] = function(test)
  coroutine.wrap(function()
    local path = '_test_uv.fs.write_and_read.async'
    local fd = uv.fs.open(path, 'w', '0666')
    local str = 'Hello, libuv!\n'
    local n = uv.fs.write(fd, str)
    test.equal(n, #str)
    uv.fs.close(fd)

    fd = uv.fs.open(path, 'r')
    local buf = Buffer.new(#str)
    n = uv.fs.read(fd, buf)
    test.equal(buf:toString(), str)
    uv.fs.close(fd)

    uv.fs.unlink(path)

    test.done()
  end)()

  uv.run()
end

exports['uv.fs.unlink.sync'] = function(test)
  local path = '_test_uv.fs.unlink.sync'
  local fd = uv.fs.open(path, 'w', '0666')
  uv.fs.close(fd)

  uv.fs.unlink(path)

  test.ok(true)
  test.done()
end

exports['uv.fs.unlink.async'] = function(test)
  coroutine.wrap(function()
    local path = '_test_uv.fs.unlink.async'
    local fd = uv.fs.open(path, 'w', '0666')
    uv.fs.close(fd)

    uv.fs.unlink(path)

    test.ok(true)
    test.done()
  end)()

  uv.run()
end

exports['uv.fs.mkdir_rmdir.sync'] = function(test)
  local path = '_test_fs_mkdir_rmdir.sync'
  uv.fs.mkdir(path)

  local stat = uv.fs.stat(path)
  test.ok(stat:isDirectory())

  uv.fs.rmdir(path)

  test.done()
end

exports['uv.fs.mkdir_rmdir.async'] = function(test)
  coroutine.wrap(function()
    local path = '_test_fs_mkdir_rmdir.async'
    uv.fs.mkdir(path)

    local stat = uv.fs.stat(path)
    test.ok(stat:isDirectory())

    uv.fs.rmdir(path)

    test.done()
  end)()

  uv.run()
end

exports['uv.fs.rename_dir.sync'] = function(test)
  local oldPath = '_test_fs_rename_dir.sync.old'
  local newPath = '_test_fs_rename_dir.sync.new'

  test.ok(not uv.fs.exists(oldPath))
  test.ok(not uv.fs.exists(newPath))

  uv.fs.mkdir(oldPath)
  test.ok(uv.fs.exists(oldPath))
  test.ok(not uv.fs.exists(newPath))

  uv.fs.rename(oldPath, newPath)
  test.ok(not uv.fs.exists(oldPath))
  test.ok(uv.fs.exists(newPath))

  uv.fs.rmdir(newPath)

  test.done()
end

exports['uv.fs.rename_dir.async'] = function(test)
  coroutine.wrap(function()
    local oldPath = '_test_fs_rename_dir.async.old'
    local newPath = '_test_fs_rename_dir.async.new'

    test.ok(not uv.fs.exists(oldPath))
    test.ok(not uv.fs.exists(newPath))

    uv.fs.mkdir(oldPath)
    test.ok(uv.fs.exists(oldPath))
    test.ok(not uv.fs.exists(newPath))

    uv.fs.rename(oldPath, newPath)
    test.ok(not uv.fs.exists(oldPath))
    test.ok(uv.fs.exists(newPath))

    uv.fs.rmdir(newPath)

    test.done()
  end)()

  uv.run()
end

exports['uv.fs.ftruncate.sync'] = function(test)
  local path = '_test_fs_ftruncate.sync'
  local fd = uv.fs.open(path, 'w', '0666')
  local str = 'Hello, libuv!\n'
  local n = uv.fs.write(fd, str)

  uv.fs.ftruncate(fd, 5)

  local stat = uv.fs.stat(path)
  test.equal(stat:size(), 5)

  uv.fs.ftruncate(fd)

  stat = uv.fs.stat(path)
  test.equal(stat:size(), 0)

  uv.fs.close(fd)

  uv.fs.unlink(path)

  test.done()
end

exports['uv.fs.ftruncate.async'] = function(test)
  coroutine.wrap(function()
    local path = '_test_fs_ftruncate.async'
    local fd = uv.fs.open(path, 'w', '0666')
    local str = 'Hello, libuv!\n'
    local n = uv.fs.write(fd, str)

    uv.fs.ftruncate(fd, 5)

    local stat = uv.fs.stat(path)
    test.equal(stat:size(), 5)

    uv.fs.ftruncate(fd)

    stat = uv.fs.stat(path)
    test.equal(stat:size(), 0)

    uv.fs.close(fd)

    uv.fs.unlink(path)

    test.done()
  end)()

  uv.run()
end

return exports
