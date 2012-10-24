local uv = require 'couv'

local TEST_PORT = 9123

local targetConnections
local MAX_SIMULTANEOUS_CONNECTS = 100

local PRINT_STATS = false
local STATS_INTERVAL = 1000 -- msec
local STATS_COUNT = 5

local WRITE_BUFFER_SIZE = 8192
local writeBuffer = uv.Buffer.new(WRITE_BUFFER_SIZE)

local maxReadSockets = 0
local maxConnectSocket = 0
local readSockets = 0
local writeSockets = 0

local nrecv = 0
local nrecvTotal = 0
local nsent = 0
local nsentTotal = 0

local statsLeft = 0

local MAX_WRITE_HANDLES = 1000

local streamType
local connectAddr
local tcpWriteHandles = {}
local pipeWriteHandles = {}

local startTime
local timerHandle

function gbit(bytes, passedMs)
  local gbits = bytes / (1024 * 1024 * 1024) * 8
  return gbits / passedMs / 1000
end

function readShowStats()
  uv.update_time()
  local diff = nv.now() - startTime
  print(string.format("%s_pump%d_server: %.1f gbits/s", streamType,
      maxReadSockets, gbit(nrecvTotal, diff)))
end

function connectionCb(server)
  coroutine.wrap(function()
    local ok, err = pcall(function()
    local stream
    if streamType == 'tcp' then
      stream = uv.tcp_create()
    else
      stream = uv.pipe_create()
    end
    uv.accept(server, stream)
    uv.read_start(stream)
    readSockets = readSockets + 1

    maxReadSockets = maxReadSockets + 1

    if nrecvTotal == 0 then
log("connectCb#1")
      assert(startTime == nil)
      uv.update_time()
      startTime = uv.now()
    end

    repeat
      local nread, buf = uv.read(stream)
      if nread and nread > 0 then
        nrecv = nrecv + nread
        nrecvTotal = nrecvTotal + nread
log("connectCb#2 nread=", nread, "nrecv", nrecv)
      end
    until nread and nread < 0
    uv.close(stream)
    readSockets = readSockets - 1
    if uv.now() - startTime > 1000 and readSockets then
log("connectCb#4")
      readShowStats()
      uv.close(server)
log("connectCb#5")
    end
log("connectCb#6")
  end)
  if err then
    log(err)
  end
  end)()
end

function tcpPumpServer()
  coroutine.wrap(function()
    local ok, err = pcall(function()
      streamType = 'tcp'
      local listenAddr = uv.ip4addr('0.0.0.0', TEST_PORT)
      local handle = uv.tcp_create()
      uv.tcp_bind(handle, listenAddr)
      uv.listen(handle, MAX_WRITE_HANDLES, connectionCb)
    end)
    if err then
      log("tcpPumpServer err " .. err)
    end
  end)()
end

function showStats()
  if printStats then
    print(string.format("connections: %d, write %.1f gbits/s", writeSockets,
        gbit(nsent, STATS_INTERNAL)))
  end

  -- Exit if the show is over
  statsLeft = statsLeft - 1
  if statsLeft == 0 then
    uv.update_time()
    local diff = uv.now() - startTime
    print(string.format("%s_pump%d_client: %.1f gbits/s", streamType,
        writeSockets, gbit(nsentTotal, diff)))
    for i = 1, writeSockets do
      uv.close(streamType == 'tcp' and
          tcpWriteHandles[i] or pipeWriteHandles[i])
    end

    os.exit(0)
  end

  -- Reset read and write counters
  nrecv = 0
  nsent = 0
end

function startStatsCollection()
  -- Show-stats timer
print("startStatsCollection start")
  statsLeft = STATS_COUNT
  timerHandle = uv.timer_create()
  uv.timer_start(timerHandle, showStats, STATS_INTERVAL, STATS_INTERVAL)
  uv.update_time()
  startTime = uv.now()
print("startStatsCollection end")
end

function doWrite(stream)
print("doWrite start")
  while uv.get_write_queue_size(stream) == 0 do
    uv.write(stream, {writeBuffer})
  end
  nsent = nsent + #writeBuffer
  nsentTotal = nsentTotal + #writeBuffer
  doWrite(stream)
print("doWrite end")
end

function onConnect()
  writeSockets = writeSockets + 1
log("onConnect", writeSockets, targetConnections)
  coroutine.wrap(function()
    mayConnectSome()
  end)()
  if writeSockets == targetConnections then
    startStatsCollection()

    -- Yay! start writing
    for i = 1, writeSockets do
      doWrite(streamType == 'tcp' and tcpWriteHandles[i] or pipeWriteHandles[i])
    end
  end
end

function maybeConnectSome()
log("maybeConnectSome targetConenections", targetConnections)
  while (maxConnectSocket < targetConnections and
      maxConnectSocket < writeSockets + MAX_SIMULTANEOUS_CONNECTS) do
    if streamType == 'tcp' then
      maxConnectSocket = maxConnectSocket + 1
log("maybeConnectSome#2 maxConnectSocket", maxConnectSocket)
      local handle = uv.tcp_create()
      tcpWriteHandles[maxConnectSocket] = handle
      uv.tcp_connect(handle, connectAddr)
      onConnect()
    end
  end
end

function tcpPump(n)
  assert(n <= MAX_WRITE_HANDLES)
  targetConnections = n
  streamType = 'tcp'
  connectAddr = uv.ip4addr('127.0.0.1', TEST_PORT)
  coroutine.wrap(function()
    maybeConnectSome()
  end)()
  uv.run()
end

function spawnServerAndRunClient(streamType, func)
  function exitCb(process, exitStatus, termSignal)
    uv.close(process)
  end
  local process = uv.spawn{
      args={uv.exepath(), arg[0], streamType .. '_pump_server'},
      flags=uv.PROCESS_DETACHED,
      exitCb=exitCb}

  local timer = uv.timer_create()
  uv.timer_start(timer, func, 10)
end

function log(...)
  local file = io.open('benchmark-pump.log', 'a+')
  file:write(os.date('%c'), ..., '\n')
  file:close()
--[[
  local fd = uv.fs_open('benchmark-pump.log', 'a+', '644')
  local line = msg .. '\n'
  uv.fs_write(fd, line)
  uv.fs_close(fd)
--]]
end

if arg[1] == 'tcp_pump_server' then
log("tcp_pump1_server")
  tcpPumpServer()
  uv.run()
  os.exit(0)
end

if arg[1] == 'tcp_pump1_client' then
log("tcp_pump1_client")
  spawnServerAndRunClient('tcp', function()
    tcpPump(1)
  end)
  uv.run()
end
