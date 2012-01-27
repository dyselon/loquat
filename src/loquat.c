#include "loquat.h"

static const struct luaL_Reg loquat_public [] = {
  { NULL, NULL }
};

int SHARED loq_openlib(lua_State *L)
{
  luaL_newlib(L, loquat_public);
  return 1;
}