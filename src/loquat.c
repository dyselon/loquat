#include "loquat.h"
#include "uv.h"
#include "timer.h"

static int loq_run(lua_State *L);

static const struct luaL_Reg loquat_public [] = {
  { "timer", loq_timer },
  { "run", loq_run },
  { NULL, NULL }
};

int SHARED loq_openlib(lua_State *L)
{
  luaL_newlib(L, loquat_public);
  return 1;
}

static int loq_run(lua_State *L)
{
  uv_loop_t* loop = uv_default_loop();
  uv_run (loop);
}