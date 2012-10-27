if arg[1] == 'spawn_helper1' then
  os.exit(1)
end

if arg[1] == 'spawn_helper2' then
  print("hello world")
  os.exit(1)
end

if arg[1] == 'spawn_helper3' then
  local uv = require 'couv'
  local buf = uv.Buffer.new(256)
  local nread = uv.fs.read(0, buf)
  uv.fs.write(1, buf, 1, nread)
  uv.run()
  os.exit(1)
end

if arg[1] == 'spawn_helper5' then
  local uv = require 'couv'
  uv.fs.write(3, 'fourth stdio!\n')
  uv.run()
  os.exit(1)
end

if arg[1] == 'spawn_helper4' then
  -- Never surrender, never return!
  local uv = require 'couv'
  function cb(handle)
  end
  local timer = uv.Timer.new()
  timer:start(cb, 10000, 10000)
  uv.run()
end

if arg[1] == 'spawn_helper6' then
  io.stderr:write('hello world\n')
  io.stderr:write('hello errworld\n')
  os.exit(1)
end
