local loquat = require("loquat")

for k, v in pairs(loquat) do
  print(tostring(k) .. ": " .. tostring(v))
end

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

loquat.run()