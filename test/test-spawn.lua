local uv = require 'couv'
local Buffer = uv.Buffer

local exports = {}

exports['spawn.fails'] = function(test)
  function exitCbFailureExpected(process, exitStatus, termSignal)
    test.equal(exitStatus, -1)
    test.equal(termSignal, 0)
    uv.close(process)
  end
  local process = uv.spawn{args={'program-that-had-better-not-exist'},
      exitCb=exitCbFailureExpected}
  test.ok(uv.is_active(process))
  uv.run()
  test.done()
end

exports['spawn.exitCode'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    uv.close(process)
  end
  local process = uv.spawn{args={uv.exepath(), 'spawn_helper1'}, exitCb=exitCb}
  test.ok(uv.is_active(process))
  uv.run()
  test.done()
end

--[[
exports['spawn.stdout'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    uv.close(process)
  end
  local pipe = uv.pipe_create()
  local process = uv.spawn{args={uv.exepath(), 'spawn_helper2'},
      stdio={{uv.IGNORE}, {uv.CREATE_PIPE | uv.WRITABLE_PIPE, pipe}},
      exitCb=exitCb}
  test.ok(uv.is_active(process))
  uv.run()
  test.done()
end
--]]

exports['spawn.stdout_to_file'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    uv.close(process)
  end

  local output = Buffer.new(1024)
  local fd = uv.fs_open('stdout_file', 'w+', uv.S_IREAD + uv.S_IWRITE)
  local process = uv.spawn{
    args={uv.exepath(), 'test/helper.lua', 'spawn_helper2'},
    stdio={{uv.IGNORE}, {uv.INHERIT_FD, fd}}, exitCb=exitCb}
  test.ok(uv.is_active(process))
  uv.run()

  local bytesRead = uv.fs_read(fd, output, 1, #output, 0)
  test.equal(bytesRead, 12)
  uv.fs_close(fd)
  uv.fs_unlink('stdout_file')
  test.done()
end

return exports
