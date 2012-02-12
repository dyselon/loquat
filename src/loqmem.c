#include "loqmem.h"
#include "stdlib.h"

void *loq_malloc(size_t size)
{
  return malloc(size);
}

void loq_free(void *ptr)
{
  free(ptr);
}

uv_buf_t simple_alloc_cb(uv_handle_t* handle, size_t suggested_size)
{
  uv_buf_t buf;
  buf.base = loq_malloc(suggested_size);
  buf.len = suggested_size;
  return buf;
}