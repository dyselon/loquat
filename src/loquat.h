#pragma once

#ifdef BUILDDLL
#define SHARED __declspec(dllexport)
#else
#define SHARED __declspec(dllimport)
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int SHARED loq_openlib(lua_State *L);
