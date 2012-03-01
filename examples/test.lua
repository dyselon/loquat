local loquat = require("loquat")

--[[
for k, v in pairs(loquat) do
  print(tostring(k) .. ": " .. tostring(v))
end
--]]

--[[
local function wait(ms)
  coroutine.yield(function(next)
    loquat.timer(ms, function()
      next()
    end)
  end)
end

local function printstuff()
  print("Immediate")
  wait(1000)
  print("A second later")
  wait(4000)
  print("Now five seconds")
  wait(2500)
  print("7.5 seconds total!")
end

local function coresume(coro)
  success, continue = coroutine.resume(coro)
  if success and coroutine.status(coro) == "suspended" and type(continue) == "function" then
    continue(function()
      coresume(coro)
    end)
  end
end

local coro = coroutine.create(printstuff)
coresume(coro)

loquat.run()
--]]

--[[
loquat.tcpconnection("74.125.224.178", 80, function(err, socket)
  print("Connection!")
  if err then
    print("Error! " .. err)
  else
    socket:onread(function(data)
      print("Data!\n" .. data)
      --socket:close()
    end)
    socket:write("GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n")
  end
  loquat.timer(2000, function() socket:close() end)
end)
--]]

--[[
local function printaddrs(status, addrs)
  if status then
    print("OHNOS")
  else
    for _, v in ipairs(addrs) do
      print(v)
    end
  end
end

local function getaddr(host)
  print("Firing off " .. host)
  loquat.getaddrinfo(host, function(status, addrs)
    print(host)
    printaddrs(status, addrs)
  end)
end

getaddr("www.google.com")
getaddr("google.com")
getaddr("dyselon.com")
getaddr("ihopweirhwoh")
getaddr("asdljfa.youtube.com")
--]]

---[[
local function wait(ms)
  coroutine.yield(function(continue)
    loquat.timer(ms, function()
      continue()
    end)
  end)
end

local function getaddr(host)
  return coroutine.yield(function(continue)
    loquat.getaddrinfo(host, function(status, addrs)
      continue(status, addrs)
    end)
  end)
end

local function makehttprequest(hostip)
  return coroutine.yield(function(continue)
    local alldata
    loquat.tcpconnection(hostip, 80, function(err, socket)
      if err then
        continue(false, nil)
      else
        socket:onread(function(newdata)
          alldata = alldata .. newdata
          -- Ideally, we should do some, I don't know, parsing or whatever.
        end)
        socket:onclose(function()
          continue(true, alldata)
        end)
        loquat.timer(2000, function()
          socket:close()
        end)
      end
    end)      
  end)
end

local function coresume(coro, ...)
  success, continue = coroutine.resume(coro, ...)
  if success and coroutine.status(coro) == "suspended" and type(continue) == "function" then
    continue(function(...)
      coresume(coro, ...)
    end)
  end
end

local function completehttprequest(host)
  local status, addrs = getaddr(host)
  if status then print("Failed to ip for " .. host) end
  if #addrs == 0 then print("Failed to any valid ips for " .. host) end
  local success, data = makehttprequest(addrs[1])
  if not success then print("Failed to actually talk to " .. host) end
  print("Success for " .. host)
end

local function httpcoro(host)
  coresume(coroutine.create(function()
    completehttprequest(host)
  end))
end

httpcoro("www.google.com")
httpcoro("google.com")
httpcoro("dyselon.com")
httpcoro("ihopweirhwoh")
httpcoro("asdljfa.youtube.com")

loquat.run()