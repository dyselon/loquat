#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "timer.h"
#include "uv.h"
#include <stdlib.h>

typedef struct {
  uv_timer_t handle;
  lua_State* L;
} loq_timer_t;

static void timer_cb(uv_timer_t* handle, int status);

int loq_timer(lua_State *L)
{
  int time = luaL_checkinteger(L, 1);
  
  uv_loop_t* loop = uv_default_loop();
  
  // Create a timer callback and fill it up with what we need.
  loq_timer_t *timer = (loq_timer_t*)malloc(sizeof(loq_timer_t));
  uv_timer_init(loop, &timer->handle);
  timer->L = L;
  timer->handle.data = timer;
  
  // Stash the callback in lua.
  lua_pushlightuserdata(L, timer);
  lua_pushvalue(L, 2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  
  // Tell uv we want the timer to start
  uv_timer_start(&timer->handle, timer_cb, time, 0);
  
  return 0;
}

static void timer_close_cb(uv_handle_t* handle) {
  printf("Close callback\n");
  loq_timer_t *timer = (loq_timer_t*)handle->data;
  free(timer);
}

static void timer_cb(uv_timer_t* handle, int status)
{
  loq_timer_t *timer = (loq_timer_t*)handle->data;
  lua_State *L = timer->L;
  
  uv_close((uv_handle_t*)handle, timer_close_cb);
  
  lua_pushlightuserdata(L, timer);  // 1 - timer lightuserdata
  lua_rawget(L, LUA_REGISTRYINDEX); // 1 - Callback
  lua_pushlightuserdata(L, timer);  // 2 - timer lightuserdata
  lua_pushnil(L);                   // 3 - nil
  lua_rawset(L, LUA_REGISTRYINDEX); // Pops timer pointer and nil, leaving the callback back at the top of the stack.
  lua_call(L, 0, 0);
}