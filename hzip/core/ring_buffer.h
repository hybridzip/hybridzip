#include <malloc.h>
#include "../other/platform.h"

struct ringbuffer {
	size_t *buf, size;
	int front, rear;
	int offset;
	int index;
};

HZIP_FORCED_INLINE void rbuf_init(ringbuffer *buf);
HZIP_FORCED_INLINE void rbuf_insert(ringbuffer *buf, size_t obj);
HZIP_FORCED_INLINE size_t rbuf_read(ringbuffer *buf);
HZIP_FORCED_INLINE void rbuf_delete(ringbuffer *buf);
