local uv = require 'couv'
local Buffer = uv.Buffer
local P = uv.Process

local exports = {}

exports['spawn.fails'] = function(test)
  function exitCbFailureExpected(process, exitStatus, termSignal)
    test.equal(exitStatus, -1)
    test.equal(termSignal, 0)
    process:close()
  end
  local process = uv.Process.spawn{args={'program-that-had-better-not-exist'},
      exitCb=exitCbFailureExpected}
  test.ok(process:isActive())
  uv.run()
  test.done()
end

exports['spawn.exit_code'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    process:close()
  end
  local process = uv.Process.spawn{
    args={uv.exepath(), 'test/helper.lua', 'spawn_helper1'}, exitCb=exitCb}
  test.ok(process:isActive())
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
      process:close()
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local out = uv.Pipe.new()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper2'},
        stdio={{P.IGNORE}, {P.CREATE_PIPE + P.WRITABLE_PIPE, out}},
        exitCb=exitCb}
    test.ok(process:isActive())
    out:startRead()
    local nread, buf = out:read()
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), 'hello world\n')
    out:close()
  end)()
  uv.run()
  test.done()
end

exports['spawn.stdout_to_file'] = function(test)
  function exitCb(process, exitStatus, termSignal)
    test.equal(exitStatus, 1)
    test.equal(termSignal, 0)
    process:close()
  end

  local output = Buffer.new(1024)
  local fd = uv.fs_open('stdout_file', 'w+', '644')
  local process = uv.Process.spawn{
    args={uv.exepath(), 'test/helper.lua', 'spawn_helper2'},
    stdio={{P.IGNORE}, {P.INHERIT_FD, fd}}, exitCb=exitCb}
  test.ok(process:isActive())
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
      process:close()
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local pIn = uv.Pipe.new()
    local pOut = uv.Pipe.new()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper3'},
        stdio={{P.CREATE_PIPE + P.READABLE_PIPE, pIn},
               {P.CREATE_PIPE + P.WRITABLE_PIPE, pOut}},
        exitCb=exitCb}
    test.ok(process:isActive())
    local msg = 'hello-from-spawn_stdin\n'
    pIn:write({msg})
    pIn:close()
    pOut:startRead()
    local nread, buf = pOut:read()
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), msg)
    pOut:close()
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
      process:close()
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local pipe = uv.Pipe.new()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper5'},
        stdio={{P.IGNORE},
               {P.IGNORE},
               {P.IGNORE},
               {P.CREATE_PIPE + P.WRITABLE_PIPE, pipe}},
        exitCb=exitCb}
    test.ok(process:isActive())
    pipe:startRead()
    local nread, buf = pipe:read()
    test.ok(nread > 0)
    test.equal(buf:toString(1, nread), 'fourth stdio!\n')
    pipe:close()
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
      process:close()
    end)()
    test.ok(true)
  end

  coroutine.wrap(function()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper6'},
        exitCb=exitCb}
    test.ok(process:isActive())
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
      process:close()
    end)()
  end

  coroutine.wrap(function()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper4'},
        flags=P.DETACHED,
        exitCb=exitCb}
    test.ok(process:isActive())
    process:unref()
    uv.kill(process:getPid(), 0)
    uv.kill(process:getPid(), 15)
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
      process:close()
    end)()
  end

  coroutine.wrap(function()
    local pIn = uv.Pipe.new()
    local pOut = uv.Pipe.new()
    local pErr = uv.Pipe.new()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper4'},
        stdio={{P.CREATE_PIPE + P.READABLE_PIPE, pIn},
               {P.CREATE_PIPE + P.WRITABLE_PIPE, pOut},
               {P.CREATE_PIPE + P.WRITABLE_PIPE, pErr}},
        exitCb=exitCb}
    test.ok(process:isActive())
    local msg = "Nancy's joining me because the message this evening is " ..
                "not my message but ours."
    pIn:write({msg})
    pOut:startRead()
    pErr:startRead()
    function timerCb(handle)
      timerCbCalled = timerCbCalled + 1
      process:kill(15) -- SIGTERM
      coroutine.wrap(function()
        handle:close()
      end)()
    end
    local timer = uv.Timer.new()
    timer:start(timerCb, 500, 0)
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
      process:close()
    end)()
  end

  coroutine.wrap(function()
    local pIn = uv.Pipe.new()
    local pOut = uv.Pipe.new()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper3'},
        stdio={{P.CREATE_PIPE + P.READABLE_PIPE, pIn},
               {P.CREATE_PIPE + P.WRITABLE_PIPE, pOut}},
        exitCb=exitCb}
    test.ok(process:isActive())
    -- Sending signum == 0 should check if the child process is still alive,
    -- not kill it.
    process:kill(0)
    local msg = "TEST"
    pIn:write({msg})
    pOut:startRead()
    local nread, buf = pOut:read()
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
      process:close()
    end)()
  end

  coroutine.wrap(function()
    local process = uv.Process.spawn{
        args={uv.exepath(), 'test/helper.lua', 'spawn_helper4'},
        exitCb=exitCb}
    test.ok(process:isActive())
    -- Sending signum == 0 should check if the child process is still alive,
    -- not kill it.
    process:kill(0)
    process:kill(15) -- SIGTERM
  end)()
  uv.run()
  test.equal(exitCbCalled, 1)
  test.done()
end

return exports
