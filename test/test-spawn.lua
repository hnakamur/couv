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

exports['spawn.exit_code'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    uv.close(process)
  end
  local process = uv.spawn{
    args={uv.exepath(), 'test/helper.lua', 'spawn_helper1'}, exitCb=exitCb}
  test.ok(uv.is_active(process))
  uv.run()
  test.done()
end

exports['spawn.stdout'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    -- NOTE: If we don't wrap this in coroutine, we got the output below:
    -- spawn.stdout: 5/6 within 0.000 seconds
    -- 40.497688325337
    -- TODO: Investigate the reason
    coroutine.wrap(function()
      uv.close(process)
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local out = uv.pipe_create()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper2'},
        stdio={{uv.IGNORE}, {uv.CREATE_PIPE + uv.WRITABLE_PIPE, out}},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    uv.read_start(out)
    local nread, buf = uv.read(out)
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), 'hello world\n')
    uv.close(out)
  end)()
  uv.run()
  test.done()
end

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

exports['spawn.stdin'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    -- NOTE: If we don't wrap this in coroutine, we got the output below:
    -- spawn.stdin: 5/6 within 0.000 seconds
    -- 1194963207.2802
    -- TODO: Investigate the reason
    coroutine.wrap(function()
      uv.close(process)
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local pIn = uv.pipe_create()
    local pOut = uv.pipe_create()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper3'},
        stdio={{uv.CREATE_PIPE + uv.READABLE_PIPE, pIn},
               {uv.CREATE_PIPE + uv.WRITABLE_PIPE, pOut}},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    local msg = 'hello-from-spawn_stdin\n'
    uv.write(pIn, {msg})
    uv.close(pIn)
    uv.read_start(pOut)
    local nread, buf = uv.read(pOut)
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), msg)
    uv.close(pOut)
  end)()
  uv.run()
  test.done()
end

exports['spawn.stdio_greater_than_3'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    -- NOTE: If we don't wrap this in coroutine, we got the output below:
    -- spawn.stdio_greater_than_3: 5/6 within 0.000 seconds
    -- 12.871675496217
    -- TODO: Investigate the reason
    coroutine.wrap(function()
      uv.close(process)
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local pipe = uv.pipe_create()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper5'},
        stdio={{uv.IGNORE},
               {uv.IGNORE},
               {uv.IGNORE},
               {uv.CREATE_PIPE + uv.WRITABLE_PIPE, pipe}},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    uv.read_start(pipe)
    local nread, buf = uv.read(pipe)
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), 'fourth stdio!\n')
    uv.close(pipe)
  end)()
  uv.run()
  test.done()
end

exports['spawn.ignored_stdio'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    -- NOTE: If we don't wrap this in coroutine, we got the output below:
    -- spawn.ignored_stdio: 3/4 within 0.000 seconds
    -- 1207684.2546877
    -- TODO: Investigate the reason
    coroutine.wrap(function()
      uv.close(process)
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper6'},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
  end)()
  uv.run()
  test.done()
end

exports['spawn.detached'] = function(test)
  local exitCbCalled = 0
  function exitCb(process, exitStatus, termSignal)
    exitCbCalled = exitCbCalled + 1
    test.equal(exitStatus, 0)
    test.equal(termSignal, 15)
    -- NOTE: If we don't wrap this in coroutine, we got the output below:
    -- spawn.detached: 3/4 within 0.000 seconds
    -- 19926532.090693
    -- TODO: Investigate the reason
    coroutine.wrap(function()
      uv.close(process)
    end)()
  end

  coroutine.wrap(function()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper4'},
        flags=uv.PROCESS_DETACHED,
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    uv.unref(process)
    uv.kill(uv.get_pid(process), 0)
    uv.kill(uv.get_pid(process), 15)
  end)()
  uv.run()
  test.equal(exitCbCalled, 1)
  test.done()
end

exports['spawn.kill_with_std'] = function(test)
  local exitCbCalled = 0
  local timerCbCalled = 0
  function exitCb(process, exitStatus, termSignal)
    exitCbCalled = exitCbCalled + 1
    test.equal(exitStatus, 0)
    test.equal(termSignal, 15)
    coroutine.wrap(function()
      uv.close(process)
    end)()
  end

  coroutine.wrap(function()
    local pIn = uv.pipe_create()
    local pOut = uv.pipe_create()
    local pErr = uv.pipe_create()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper4'},
        stdio={{uv.CREATE_PIPE + uv.READABLE_PIPE, pIn},
               {uv.CREATE_PIPE + uv.WRITABLE_PIPE, pOut},
               {uv.CREATE_PIPE + uv.WRITABLE_PIPE, pErr}},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    local msg = "Nancy's joining me because the message this evening is " ..
                "not my message but ours."
    uv.write(pIn, {msg})
    uv.read_start(pOut)
    uv.read_start(pErr)
    function timerCb(handle)
      timerCbCalled = timerCbCalled + 1
      uv.process_kill(process, 15) -- SIGTERM
      coroutine.wrap(function()
        uv.close(handle)
      end)()
    end
    local timer = uv.timer_create()
    uv.timer_start(timer, timerCb, 500, 0)
  end)()
  uv.run()
  test.equal(exitCbCalled, 1)
  test.equal(timerCbCalled, 1)
  test.done()
end

exports['spawn.ping'] = function(test)
  local exitCbCalled = 0
  function exitCb(process, exitStatus, termSignal)
    exitCbCalled = exitCbCalled + 1
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    coroutine.wrap(function()
      uv.close(process)
    end)()
  end

  coroutine.wrap(function()
    local pIn = uv.pipe_create()
    local pOut = uv.pipe_create()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper3'},
        stdio={{uv.CREATE_PIPE + uv.READABLE_PIPE, pIn},
               {uv.CREATE_PIPE + uv.WRITABLE_PIPE, pOut}},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    -- Sending signum == 0 should check if the child process is still alive,
    -- not kill it.
    uv.process_kill(process, 0)
    local msg = "TEST"
    uv.write(pIn, {msg})
    uv.read_start(pOut)
    local nread, buf = uv.read(pOut)
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), msg)
  end)()
  uv.run()
  test.equal(exitCbCalled, 1)
  test.done()
end

exports['spawn.kill'] = function(test)
  local exitCbCalled = 0
  function exitCb(process, exitStatus, termSignal)
    exitCbCalled = exitCbCalled + 1
    test.equal(exitStatus, 0)
    test.equal(termSignal, 15)
    coroutine.wrap(function()
      uv.close(process)
    end)()
  end

  coroutine.wrap(function()
    local process = uv.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper4'},
        exitCb=exitCb}
    test.ok(uv.is_active(process))
    -- Sending signum == 0 should check if the child process is still alive,
    -- not kill it.
    uv.process_kill(process, 0)
    uv.process_kill(process, 15) -- SIGTERM
  end)()
  uv.run()
  test.equal(exitCbCalled, 1)
  test.done()
end

return exports
