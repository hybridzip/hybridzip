#include "ring_buffer.h"

//does not include error handling, use it at your own risk.

HZIP_FORCED_INLINE void rbuf_init(ringbuffer *buf) {
	buf->front = buf->rear = -1;
	buf->index = 0;
	buf->buf = (size_t*)malloc(sizeof(size_t) * buf->size);
}

HZIP_FORCED_INLINE void rbuf_insert(ringbuffer* buf, size_t obj) {
	if (buf->front == -1 && buf->rear == -1) {
		buf->front = buf->rear = 0;
		buf->buf[0] = obj;
	} else if((buf->rear == buf->front - 1) || (buf->front == 0 && buf->rear == buf->size - 1)){
		// buffer-overflow
	} else {
		buf->rear = (buf->rear + 1) % buf->size;
		buf->buf[buf->rear] = obj;
	}
}

HZIP_FORCED_INLINE size_t rbuf_read(ringbuffer* buf) {
	size_t retval = buf->buf[buf->front + buf->offset];
	buf->offset = (buf->offset + 1) % buf->size;
	return retval;
}

HZIP_FORCED_INLINE void rbuf_delete(ringbuffer* buf) {
	buf->offset = 0;
	buf->front = (buf->front + 1) % buf->size;
}