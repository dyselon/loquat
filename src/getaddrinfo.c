#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "uv.h"
#include "getaddrinfo.h"
#include <stdlib.h>

void getaddrinfo_cb(uv_getaddrinfo_t* handle, int status, struct addrinfo* res);

extern int loq_getaddrinfo(lua_State *L)
{
  uv_loop_t* loop = uv_default_loop();
  const char* addr = luaL_checkstring(L, 1);
  luaL_checkany(L, 2);
  
  uv_getaddrinfo_t *handle = (uv_getaddrinfo_t *)loq_malloc(sizeof(uv_getaddrinfo_t));
  handle->data = L;
  lua_pushlightuserdata(L, handle);
  lua_pushvalue(L, 2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  int r = uv_getaddrinfo(loop, handle, getaddrinfo_cb, addr, NULL, NULL);
    
  if(r) {
    lua_pushboolean(L, 0);
    return 1;
  }

  lua_pushboolean(L, 1);
  return 1;
}

void getaddrinfo_cb(uv_getaddrinfo_t* handle, int status, struct addrinfo* res)
{
  lua_State *L = handle->data;
	lua_pushlightuserdata(L, handle);
	lua_rawget(L, LUA_REGISTRYINDEX);
	
  if(!lua_isnil(L, -1))
  {
    if(status)
    {
      lua_pushinteger(L, status);
      lua_pushnil(L);
      if(lua_pcall(L, 2, 0, 0))
      {
        printf("Lua error doing connect cb - %s\n", lua_tostring(L, -1));
      }
    }
    else
    {
      lua_pushnil(L);
      lua_newtable(L);
      int i = 1;
      while(res)
      {
        if(res->ai_family == AF_INET)
        {
          struct sockaddr_in *inetaddr = (struct sockaddr_in*)res->ai_addr;
          char ipstr[16];
          uv_ip4_name(inetaddr, ipstr, 16);
          lua_pushstring(L, ipstr);
          lua_rawseti(L, -2, i++);
        }
        res = res->ai_next;
      }
      if(lua_pcall(L, 2, 0, 0))
      {
        printf("Lua error doing connect cb - %s\n", lua_tostring(L, -1));
      }
    }
  }
  uv_freeaddrinfo(res);
}
