#include "loquat.h"
#include "uv.h"

#include "timer.h"
#include "tcpsocket.h"
#include "getaddrinfo.h"
#include "buffer.h"

static int loq_run(lua_State *L);

static const struct luaL_Reg loquat_public [] = {
  { "timer",          loq_timer         },
  { "tcpconnection",  loq_tcpconnection },
  { "getaddrinfo",    loq_getaddrinfo   },
  { "buffer",         loq_buffer        },
  { "run",            loq_run           },
  { NULL, NULL }
};

int SHARED loq_openlib(lua_State *L)
{
  loq_tcpconnection_init(L);
  loq_buffer_init(L);
  luaL_newlib(L, loquat_public);
  return 1;
}

static int loq_run(lua_State *L)
{
  uv_loop_t* loop = uv_default_loop();
  uv_run (loop);
}