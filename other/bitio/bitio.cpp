/*
MIT License

Copyright (c) 2020 Vishaal Selvaraj

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */


#include "bitio.h"

using namespace bitio;

bitio_stream::bitio_stream(std::string filename, access_enum op, bool in_mem, uint64_t buffer_size) {
    file = nullptr;
    this->filename = filename;
    mode = op;

    if ((op == READ) && !(file = fopen(filename.c_str(), "rb"))) {
        return;
    } else if (op == WRITE) {
        file = fopen(filename.c_str(), "wb");
    } else if (op == APPEND) {
        file = fopen(filename.c_str(), "a");
    }

    this->buffer_size = buffer_size;
    byte_buffer = (unsigned char *) malloc(sizeof(unsigned char) * buffer_size);
    bit_count = 0;
    eof = false;
    bit_buffer = 0;
    current_buffer_length = 0;
    byte_index = 0;

    if (in_mem) {
        move_to_memory();
    }
}


void bitio_stream::close() {
    free(byte_buffer);
    if (file != nullptr) fclose(file);
}

inline void bitio_stream::load_buffer() {
    auto tmp_len = fread(byte_buffer, 1, buffer_size, file);
    current_buffer_length = tmp_len != 0 ? tmp_len : current_buffer_length;
    eof = tmp_len == 0;
    byte_index = 0;
}

inline void bitio_stream::load_byte() {
    if (eof)
        bit_buffer = 0;
    if (byte_index == current_buffer_length)
        load_buffer();
    bit_buffer = byte_buffer[byte_index++];
}

inline void bitio_stream::wflush() {
    fwrite(byte_buffer, 1, buffer_size, file);
}

uint64_t bitio_stream::read(uint8_t n) {
    if (n == 0) {
        return 0;
    }

    uint64_t value = 0;
    if (bit_count == 0) {
        load_byte();
        bit_count = 8;
    }

    if (bit_count >= n) {
        value += (bit_buffer & reverse_bit_masks[n]) >> (8 - n);
        bit_buffer <<= n;
        bit_count -= n;
    } else {
        value += bit_buffer >> (8 - bit_count);
        char target_bits = n - bit_count;
        char nbytes = target_bits >> 3;
        bit_buffer = 0;
        bit_count = 0;

        while (nbytes--) {
            load_byte();
            value <<= 8;
            value += bit_buffer;
        }
        load_byte();
        bit_count = 8;

        char rembits = target_bits & bit_masks[3];

        value <<= rembits;
        value += (bit_buffer & reverse_bit_masks[rembits]) >> (8 - rembits);
        bit_buffer <<= rembits;
        bit_count -= rembits;
    }
    return value & ui64_masks[n];
}

void bitio_stream::seek(int64_t n) {
    if (n == 0) return;
    if (n > 0) {
        skip(n);
    } else {

        auto nbits = -n;
        auto nbytes = nbits >> 0x3;
        auto rbits = nbits & 0x7;

        int64_t al_offset = (int64_t) byte_index - (int64_t) nbytes - 1 - (int64_t) current_buffer_length;

        fseek(file, al_offset, SEEK_CUR);

        load_buffer();
        byte_index++;

        if (rbits <= 8 - bit_count) {
            bit_count += rbits;
            bit_buffer = byte_buffer[byte_index - 1];
            bit_buffer <<= 8 - bit_count;
        } else {
            bit_buffer = byte_buffer[--byte_index];
            bit_buffer <<= 8 - bit_count;

            if (byte_index == 0) {
                fseek(file, -1 - (int64_t) current_buffer_length, SEEK_CUR);
                load_buffer();
                bit_buffer = byte_buffer[byte_index++];
                bit_buffer <<= 8 - bit_count;
            }

            lim_skip(8 - rbits);
        }

    }
}

// wrapper for lim_skip to allow skips beyond 64-bits.
void bitio_stream::skip(uint64_t n) {
    while (n > 0) {
        if (n > 0x40) {
            lim_skip(0x40);
            n -= 0x40;
        } else {
            lim_skip(n);
            n = 0;
        }
    }
}

void bitio_stream::write(uint64_t obj, uint64_t n) {
    uint64_t i = 0;
    obj <<= 0x40 - n;

    unsigned char mask_index = 0;
    while (i++ < n) {
        bit_buffer <<= 1;
        bit_buffer += (obj & ui64_single_bit_masks[0x3f - mask_index++]) != 0;;
        bit_count++;
        if (bit_count == 8) {
            byte_buffer[byte_index++] = bit_buffer;
            if (byte_index == buffer_size) {
                wflush();
                byte_index = 0;
            }
            bit_buffer = 0;
            bit_count = 0;
        }

    }
}

// aligns to next-byte, meaningful only for read operations.
void bitio_stream::align() {
    lim_skip(bit_count);
    //load_byte();
}

// write residues and align to next-byte.
void bitio_stream::flush() {
    if (bit_count != 0) {
        bit_buffer <<= 8 - bit_count;
        byte_buffer[byte_index++] = bit_buffer;
    }
    fwrite(byte_buffer, 1, byte_index, file);
}

bool bitio_stream::is_eof() {
    if (eof) return true;
    if (byte_index == current_buffer_length) {
        auto *tmp_ptr = new char[1];
        eof = fread(tmp_ptr, 1, 1, file) == 0;
        free(tmp_ptr);

        if (!eof) fseek(file, -1, SEEK_CUR);
    }
    return eof;
}

uint64_t bitio_stream::get_file_size() {
    FILE *tmp = fopen(filename.c_str(), "rb");
    uint64_t count = 0;
    char *tmp_ptr = new char[1];
    while (fread(tmp_ptr, 1, 1, tmp) != 0) {
        count++;
    }
    free(tmp_ptr);
    fclose(tmp);
    return count;
}

inline void bitio_stream::lim_skip(uint8_t n) {
    if (bit_count == 0) {
        load_byte();
        bit_count = 8;
    }

    if (bit_count >= n) {
        bit_buffer <<= n;
        bit_count -= n;
    } else {
        char target_bits = n - bit_count;
        char nbytes = target_bits >> 3;
        bit_buffer = 0;
        bit_count = 0;

        while (nbytes--) {
            load_byte();
        }
        load_byte();
        bit_count = 8;

        char rembits = target_bits & bit_masks[3];
        bit_buffer <<= rembits;
        bit_count -= rembits;
    }
}

uint64_t bitio_stream::peek(uint8_t n) {
    uint64_t peek_val = read(n);
    seek(-n);
    return peek_val;
}

void bitio_stream::force_align() {
    if (bit_count == 0) {
        lim_skip(0x8);
    } else {
        lim_skip(bit_count);
    }
}

void bitio_stream::move_to_memory() {
    // Make file to point to data in RAM instead of Secondary storage.
    // Move entire file to RAM from SS.
    is_inmem = true;
    uint64_t file_size = get_file_size();
    inmem_cache = new uint8_t[file_size];

    fread(inmem_cache, 1, file_size, file);
    fclose(file);
    if (mode == READ) {
        file = fmemopen(inmem_cache, file_size, "rb");
    } else if (mode == WRITE) {
        file = fmemopen(inmem_cache, file_size, "wb");
    } else if (mode == APPEND) {
        file = fmemopen(inmem_cache, file_size, "a");
    }
}

bitio_stream::~bitio_stream() {
    if (is_inmem) {
        free(inmem_cache);
    }
    free(byte_buffer);
}


