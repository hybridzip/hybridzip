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

	struct BitArray {
		bool *bitset;
		size_t length;
	};

	class BitIO {
	private:

		FILE *file, *wfile;
		unsigned char bit_buffer, *byte_buffer;
		unsigned char bit_count;

		size_t buffer_size, byte_index, current_buffer_length;
		bool eof, read_mode;

		unsigned char wbit_buffer, *wbyte_buffer, wbit_count;
		size_t wbyte_index;

		HZIP_FORCED_INLINE void load_buffer();
		HZIP_FORCED_INLINE void load_byte();
		HZIP_FORCED_INLINE void wflush();


	public:

		BitIO(char* filename, size_t read_buffer_size);
		void skip(size_t n);
		size_t read(size_t n);
		void write(size_t obj, size_t n);
		void flush();
		void close();

	};
}
