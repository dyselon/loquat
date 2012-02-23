#include "buffer.h"
#include "lauxlib.h"
#include "string.h"
#include "stdint.h"

typedef struct {
  char *base;
  int capacity;
  int size;
  unsigned int cursor;
} loq_buf_t;

// Checks that an argument is a buffer and returns a pointer to that buffer
static loq_buf_t* loq_checkbuffer(lua_State *L, int arg);

void loq_buf_reserve(loq_buf_t* buf, size_t newcapacity);
void loq_buf_copy(loq_buf_t* buf, const void *src, size_t len);

static int loq_buf_addstring(lua_State *L);
static int loq_buf_addint32(lua_State *L);
static int loq_buf_addint64(lua_State *L);
static int loq_buf_adddouble(lua_State *L);
static int loq_buf_free(lua_State *L);

static const struct luaL_Reg loquat_buffer [] = {
  { "addstring", loq_buf_addstring },
  { "addint32",  loq_buf_addint32  },
  { "addint64",  loq_buf_addint64  },
  { "adddouble", loq_buf_adddouble },
  { "__gc",      loq_buf_free      },
  { NULL, NULL }
};

int loq_buffer(lua_State *L)
{
  int args = lua_gettop(L);
  int size = 256;
  
  loq_buf_t *buf = (loq_buf_t *)lua_newuserdata(L, sizeof(loq_buf_t));
  buf->base = (char *)loq_malloc(size);
  buf->capacity = size;
  buf->size = 0;
  return 1;
}

void loq_buffer_init(lua_State *L)
{
  luaL_newmetatable(L, "loquat.buffer");
  luaL_setfuncs(L, loquat_buffer, 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
}

static loq_buf_t* loq_checkbuffer(lua_State *L, int arg)
{
  loq_buf_t *buf = (loq_buf_t *)luaL_checkudata(L, arg, "loquat.buffer");
  return buf;
}

static int loq_buf_free(lua_State *L)
{
  loq_buf_t* buf = loq_checkbuffer(L, 1);
  if(buf->base)
  {
    loq_free(buf->base);
    buf->base = NULL;
  }
  buf->capacity = 0;
  buf->size = 0;
}

void loq_buf_reserve(loq_buf_t* buf, size_t newcapacity)
{
  if(newcapacity < buf->capacity)
    return;
  
  char *newbuf = (char *)loq_malloc(newcapacity);
  memcpy(newbuf, buf->base, buf->size);
  loq_free(buf->base);
  buf->base = newbuf;
}

void loq_buf_copy(loq_buf_t* buf, const void *src, size_t len)
{
  if(buf->capacity < buf->size + len)
  {
    loq_buf_reserve(buf, buf->size + len);
  }
  
  memcpy(buf->base + buf->size, src, len);
  buf->size += len; 
}

static int loq_buf_addstring(lua_State *L)
{
  loq_buf_t* buf = loq_checkbuffer(L, 1);
  size_t len;
  const char *str = lua_tolstring(L, 2, &len);
  
  loq_buf_copy(buf, str, len);
  
  return 0;
}

static int loq_buf_addint32(lua_State *L)
{
  loq_buf_t* buf = loq_checkbuffer(L, 1);
  int isnum;
  int32_t num = lua_tointegerx(L, 2, &isnum);
  
  if(!isnum)
    return 0;

  loq_buf_copy(buf, &num, sizeof(num));
  
  return 0;
}

static int loq_buf_addint64(lua_State *L)
{
  loq_buf_t* buf = loq_checkbuffer(L, 1);
  int isnum;
  int64_t num = lua_tointegerx(L, 2, &isnum);
  
  if(!isnum)
    return 0;

  loq_buf_copy(buf, &num, sizeof(num));
  
  return 0;
}

static int loq_buf_adddouble(lua_State *L)
{
  loq_buf_t* buf = loq_checkbuffer(L, 1);
  int isnum;
  double num = lua_tonumberx(L, 2, &isnum);
  
  if(!isnum)
    return 0;

  loq_buf_copy(buf, &num, sizeof(num));
  
  return 0;
}
