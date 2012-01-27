#include "loquat.h"

// This seems to crash on exiting the interperter if there's no real entries to this table.
// Since that won't be the case for long, hey, so what?
static const struct luaL_Reg loquat_public [] = {
  { NULL, NULL }
};

int SHARED loq_openlib(lua_State *L)
{
  luaL_newlib(L, loquat_public);
  return 1;
}