Loquat
======
Loquat is a binding of libuv for Lua 5.2

This project is just getting off the ground, so a lot of things don't work. I'm trying this whole Readme Driven Development thing, which means a lot of this document reflects how I would *like* things to look rather than how they actually do. My apologies about any confusion.

Introduction
------------
Loquat wraps the excellent [libuv](https://github.com/joyent/libuv) library, and makes it easy to do asynchronous stuff like network calls or file io without blocking. Combined with coroutines, it becomes (well, should become) easy to write performant servers in Lua.

Features
--------
When finished, Loquat should support a pretty extensive subset of libuv's features, such as
 * TCP
 * UDP
 * File I/O
 * Timers
 * Child processes
 * Support for multiple concurrent event loops
 * And I don't know, mabye more stuff

Currently Actually Working To Some Extent Or Another
----------------------------------------------------
 * Outgoing TCP Connections
 * Timers
 * DNS lookup

Some Examples
-------------

### TCP

Connecting to a TCP server
``` lua
local loquat = require("loquat")

local function onconnect(err, socket)
  socket:onread(function(data)
    print("Got a response! " .. data)
    socket:close()
  end)
  
  socket:write("Hey, here's some stuff!")
end

local client = loquat.tcpconnection("127.0.0.1", 8000, onconnect)
client.connect()

loquat.run()
```

Starting a TCP server
``` lua
local loquat = require("loquat")

local function onconnection(socket)
  socket.recv(function(data)
    print("Got a thing! " .. data)
    socket.send("Echo that! " .. data)
  end)
  
  socket.closed(function()
    print("Closed that socket down!")
  end)
end

local server = loquat.tcp("127.0.0.1", 8000, onconnection)
server.listen()

loquat.run()
```

### Timers

Here's a timer example that uses coroutines
``` lua
local loquat = require("loquat")

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
  if success and coroutine.state(coro) == "suspended" and type(continue) == "function" then
    continue(function() coresume(coro) end)
  end
end

local coro = coroutine.create(printstuff)
coresume(coro)

loquat.run()
```

Requirements
------------

To use loquat, you need a Lua 5.2 interperter, and the Loquat library.

You can download the Windows .dll from http://loquat.dyselon.com/files/loquat-win.zip

Building Loquat should be as easy as running make, but I don't know shit about writing makefiles. Right now it only builds for mingw. Sorry about that. 
