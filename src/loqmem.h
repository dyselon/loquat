#include "uv.h"

uv_buf_t simple_alloc_cb(uv_handle_t* handle, size_t size);
void *loq_malloc(size_t size);
void loq_free(void *ptr);