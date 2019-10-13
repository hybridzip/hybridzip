#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fstream>
#include <iostream>
#include <stdbool.h>
#include "../other/platform.h"
#include "../other/constants.h"

namespace bitio {

	/* The bitio_buffer is used for efficiently storing binary data
	   in memory. To hide memory allocation latency per byte the bitio_buffer uses a chain
	   of buffers.
	*/

	struct buffer_t {
		unsigned char *buffer;
		struct buffer_t *link;
	};

	

	class bitio_buffer {
	private:
		buffer_t *buf_head, *buf_tail;
		size_t buffer_size, byte_index, current_buffer_length;
		bool eof, read_mode;

		unsigned char wbit_buffer, *wbyte_buffer, wbit_count;
		size_t wbyte_index;

		HZIP_FORCED_INLINE void buffer_flush();
		void buffer_t_alloc(struct buffer_t *buf_t, HZIP_SIZE_T buffer_length);

	public:

		bitio_buffer(size_t read_buffer_size);
		void write(size_t obj, size_t n);
		void flush(FILE* file);
	};

}