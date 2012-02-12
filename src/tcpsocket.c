#include "lauxlib.h"
#include "loqmem.h"
#include "uv.h"
#include <stdlib.h>
#include <string.h>

#include "tcpsocket.h"

typedef struct {
  uv_tcp_t handle;

  lua_State* L;

  int connreadref;
  uv_connect_t conn_req;

  int writeref;
  uv_write_t write_req;
  
  int closeref;
  BOOL closed;
} loq_tcpconn_t;

// Checks that an argument is a tcpconnection and returns a pointer
static loq_tcpconn_t* loq_checktcpconn(lua_State *L, int arg);

// TCP Connection methods
static int loq_tcpc_onread(lua_State *L);
static int loq_tcpc_onclose(lua_State *L);
static int loq_tcpc_write(lua_State *L);
static int loq_tcpc_close(lua_State *L);

static const struct luaL_Reg loquat_tcpconnection [] = {
  { "onread",  loq_tcpc_onread  },
  { "onclose", loq_tcpc_onclose  },
  { "write",   loq_tcpc_write },
  { "close",   loq_tcpc_close },
  { "__gc",    loq_tcpc_close },
  { NULL, NULL }
};

// Necessary callbacks
void tcpc_connect_cb(uv_connect_t* req, int status);
void tcpc_read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
void tcpc_write_cb(uv_write_t* req, int status);
void tcpc_close_cb(uv_handle_t* handle);

void loq_tcpconnection_init(lua_State *L)
{
  luaL_newmetatable(L, "loquat.tcpconnection");
  luaL_setfuncs(L, loquat_tcpconnection, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
}

// TODO: Error checking on this whole mutha
int loq_tcpconnection(lua_State *L)
{
  uv_loop_t* loop = uv_default_loop();
  
  const char* ip = luaL_checkstring(L, 1);
  int port = luaL_checkinteger(L, 2);
  luaL_checkany(L, 3);
  
  // Ask lua to allocate some memory for our socket, and initialize our data
  loq_tcpconn_t *socket = (loq_tcpconn_t *)lua_newuserdata(L, sizeof(loq_tcpconn_t));
  socket->L = L;
  uv_tcp_init(loop, &socket->handle);
  socket->handle.data = socket;
  socket->handle.close_cb = tcpc_close_cb;
  socket->closed = FALSE;
  socket->writeref = LUA_NOREF;
  socket->closeref = LUA_NOREF;
  
  // Associate the correct metatable with our new userdata
  luaL_setmetatable(L, "loquat.tcpconnection");
  
  // Call uv_tcp_connect to try and actually connect to our destination
  struct sockaddr_in addr = uv_ip4_addr(ip, port);
  uv_tcp_connect(&socket->conn_req, &socket->handle, addr, tcpc_connect_cb);
  
  // Stash our lua userdata somewhere that we can get back to it.
  lua_pushlightuserdata(L, socket);
  lua_pushvalue(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  
  // Now pop the userdata off the stack. It's still here from when we originally called lua_newuserdata.
  lua_pop(L, 1);
  
  // Okay, now stash our connection callback as well.
  lua_pushvalue(L, 3);
  socket->connreadref = luaL_ref(L, LUA_REGISTRYINDEX);
  
  return 0;
}

static loq_tcpconn_t* loq_checktcpconn(lua_State *L, int arg)
{
  loq_tcpconn_t *socket = (loq_tcpconn_t *)luaL_checkudata(L, arg, "loquat.tcpconnection");
  return socket;
}

void tcpc_connect_cb(uv_connect_t* req, int status)
{
  loq_tcpconn_t *socket = (loq_tcpconn_t *)req->handle->data;
  lua_State *L = socket->L;
  
  // Push the callback function
  lua_rawgeti(L, LUA_REGISTRYINDEX, socket->connreadref);
  luaL_unref(L, LUA_REGISTRYINDEX, socket->connreadref);
  socket->connreadref = LUA_NOREF;

  // Now push the arguments - error code and nothing if we couldn't connect
  if(status)
  {
    lua_pushinteger(L, status);
    lua_pushnil(L);
  }
  // Or nil and the socket userdata if we were successful
  else
  {
    lua_pushnil(L);

    lua_pushlightuserdata(L, socket);
    lua_rawget(L, LUA_REGISTRYINDEX);
  }
  // Now call the callback function with the two arguments
  if(lua_pcall(L, 2, 0, 0))
  {
    printf("Lua error doing connect cb - %s\n", lua_tostring(L, -1));
  }
  
  // Error checking plz
  int r = uv_read_start((uv_stream_t*)&socket->handle, simple_alloc_cb, tcpc_read_cb);

  // Now clean up the references we created when calling connect
  lua_pushlightuserdata(L, socket);
  lua_pushnil(L);
  lua_rawset(L, LUA_REGISTRYINDEX);  
}

void tcpc_read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
{
  loq_tcpconn_t *socket = (loq_tcpconn_t *)stream->data;
  lua_State *L = socket->L;
  if(socket->connreadref != LUA_NOREF && socket->connreadref != LUA_REFNIL)
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, socket->connreadref);
    lua_pushlstring(L, buf.base, nread);
    if(lua_pcall(L, 1, 0, 0))
    {
      printf("Lua error doing read cb - %s\n", lua_tostring(L, -1));
    }
  }
  loq_free(buf.base);
}

static int loq_tcpc_onread(lua_State *L)
{
  loq_tcpconn_t *socket = loq_checktcpconn(L, 1);
  luaL_checkany(L, 2);
  
  lua_pushvalue(L, 2);
  socket->connreadref = luaL_ref(L, LUA_REGISTRYINDEX);
}

static int loq_tcpc_onclose(lua_State *L)
{
  loq_tcpconn_t *socket = loq_checktcpconn(L, 1);
  luaL_checkany(L, 2);
  
  lua_pushvalue(L, 2);
  socket->closeref = luaL_ref(L, LUA_REGISTRYINDEX);
}

static int loq_tcpc_write(lua_State *L)
{
  int args = lua_gettop(L);
  loq_tcpconn_t *socket = loq_checktcpconn(L, 1);
  luaL_checkany(L, 2);
  
  int len = 0;
  const char *data = lua_tolstring(L, 2, &len);
  
  if(args >= 3)
  {
    lua_pushvalue(L, 3);
    socket->writeref = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  
  uv_buf_t buf = uv_buf_init((char*)data, len);
  
  int r = uv_write(&socket->write_req, (uv_stream_t*)&socket->handle, &buf, 1, tcpc_write_cb);
}

void tcpc_write_cb(uv_write_t* req, int status)
{
  loq_tcpconn_t *socket = (loq_tcpconn_t *)req->handle->data;
  lua_State *L = socket->L;
  
  if(socket->writeref != LUA_NOREF && socket->writeref != LUA_REFNIL)
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, socket->writeref);
    lua_pushinteger(L, status);
    if(lua_pcall(L, 1, 0, 0))
    {
      printf("Lua error doing write cb - %s\n", lua_tostring(L, -1));
    }
  }
}

static int loq_tcpc_close(lua_State *L)
{
  loq_tcpconn_t *socket = loq_checktcpconn(L, 1);
  if(!socket->closed)
  {
    socket->closed = TRUE;
    uv_close((uv_handle_t*)&socket->handle, tcpc_close_cb);
  }
}

void tcpc_close_cb(uv_handle_t* handle)
{
  loq_tcpconn_t *socket = (loq_tcpconn_t *)handle->data;
  lua_State *L = socket->L;

  if(socket->closeref != LUA_NOREF && socket->closeref != LUA_REFNIL)
  {
    lua_rawgeti(L, LUA_REGISTRYINDEX, socket->closeref);
    if(lua_pcall(L, 0, 0, 0))
    {
      printf("Lua error doing close cb - %s\n", lua_tostring(L, -1));
    }
  }
  return;
}