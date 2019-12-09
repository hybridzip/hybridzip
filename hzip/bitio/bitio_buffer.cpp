#include "bitio.h"

using namespace bitio;

void bitio_buffer::buffer_t_alloc(struct buffer_t *buf_t, uint64_t buffer_length) {
	buf_t->buffer = (unsigned char*)malloc(sizeof(unsigned char) * buffer_length);
	buf_t->link = NULL;
}

bitio_buffer::bitio_buffer(uint64_t buffer_size) {
	this->buffer_size = buffer_size;
	buf_head = (buffer_t*)malloc(sizeof(buffer_t));
	buffer_t_alloc(buf_head, buffer_size);
	buf_tail = buf_head;

	wbyte_buffer = buf_head->buffer;
	current_buffer_length = 0;
	byte_index = 0;
	wbyte_index = 0;
	wbit_buffer = 0;
	wbit_count = 0;
}


void bitio_buffer::write(uint64_t obj, uint64_t n) {
	int i = 0;
	obj <<= 0x40 - n;
	unsigned char mask_index = 0;
	while (i++ != n) {
		wbit_buffer += (obj & ui64_single_bit_masks[0x3f - mask_index++]) != 0;
		wbit_count++;
		if (wbit_count == 8) {
			wbyte_buffer[wbyte_index++] = wbit_buffer;
			if (wbyte_index == buffer_size) {
				buffer_flush(); 
				wbyte_index = 0;
			}
			mask_index = 0;
			wbit_count = 0;
			obj <<= 8;
			wbit_buffer = 0;
		}
		wbit_buffer <<= 1;
	}
}

void bitio_buffer::buffer_flush() {
	struct buffer_t *new_buffer = (buffer_t*)malloc(sizeof(buffer_t));
	buffer_t_alloc(new_buffer, buffer_size);
	buf_tail->link = new_buffer;
	buf_tail = new_buffer;
	wbyte_buffer = new_buffer->buffer;
}

void bitio_buffer::flush(FILE* file) {
	if (wbit_count != 0) {
		wbit_buffer <<= 7 - wbit_count;
		wbyte_buffer[wbyte_index++] = wbit_buffer;
	}
	buffer_t* curr = buf_head;
	while (curr != buf_tail) {
		fwrite(curr->buffer, 1, buffer_size, file);
		curr = curr->link;
	}
	fwrite(curr->buffer, 1, wbyte_index, file);
}


