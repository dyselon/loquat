#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "timer.h"
#include "loqmem.h"
#include "uv.h"
#include <stdlib.h>

typedef struct {
  uv_timer_t handle;
  lua_State* L;
} loq_timer_t;

static void timer_cb(uv_timer_t* handle, int status);
static void timer_close_cb(uv_handle_t* handle);

int loq_timer(lua_State *L)
{
  int time = luaL_checkinteger(L, 1);
  int r = 0;
  
  uv_loop_t* loop = uv_default_loop();
  
  // Create a timer callback and fill it up with what we need.
  loq_timer_t *timer = (loq_timer_t*)loq_malloc(sizeof(loq_timer_t));
  r = uv_timer_init(loop, &timer->handle);
  if(r) {
    loq_free(timer);
    uv_err_t err = loop->last_err;
    return luaL_error(L, uv_strerror(err));
  }
  timer->L = L;
  timer->handle.data = timer;
  
  // Stash the callback in lua.
  lua_pushlightuserdata(L, timer);
  lua_pushvalue(L, 2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  
  // Tell uv we want the timer to start
  r = uv_timer_start(&timer->handle, timer_cb, time, 0);
  if(r) {
    uv_close((uv_handle_t*)&timer->handle, timer_close_cb);
    uv_err_t err = loop->last_err;
    return luaL_error(L, uv_strerror(err));
  }
  
  return 0;
}

static void timer_close_cb(uv_handle_t* handle) {
  loq_timer_t *timer = (loq_timer_t*)handle->data;
  loq_free(timer);
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