#include "loquat.h"
#include "tcpsocket.h"

static const struct luaL_Reg loquat_public [] = {
  { "tcpserver", loq_tcpserver },
  { NULL, NULL }
};

int SHARED loq_openlib(lua_State *L)
{
  luaL_newlib(L, loquat_public);
  return 1;
}