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

#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#ifndef BITIO_BUFFER_SIZE
#define BITIO_BUFFER_SIZE 0x400
#endif

namespace bitio {

    const unsigned char one_bit_masks[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
    const unsigned char bit_masks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
    const unsigned char reverse_bit_masks[] = {0x0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
    const size_t ui64_masks[] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f,
                                 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff,
                                 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff, 0x7ffffff,
                                 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff, 0x1ffffffff, 0x3ffffffff,
                                 0x7ffffffff,
                                 0xfffffffff, 0x1fffffffff, 0x3fffffffff, 0x7fffffffff, 0xffffffffff, 0x1ffffffffff,
                                 0x3ffffffffff, 0x7ffffffffff,
                                 0xfffffffffff, 0x1fffffffffff, 0x3fffffffffff, 0x7fffffffffff, 0xffffffffffff,
                                 0x1ffffffffffff, 0x3ffffffffffff, 0x7ffffffffffff,
                                 0xfffffffffffff, 0x1fffffffffffff, 0x3fffffffffffff, 0x7fffffffffffff, 0xffffffffffffff,
                                 0x1ffffffffffffff, 0x3ffffffffffffff, 0x7ffffffffffffff,
                                 0xfffffffffffffff, 0x1fffffffffffffff, 0x3fffffffffffffff, 0x7fffffffffffffff,
                                 0xffffffffffffffff};

    const size_t ui64_single_bit_masks[] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000,
                                            0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000,
                                            0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000,
                                            0x20000000, 0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000,
                                            0x800000000, 0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
                                            0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000, 0x100000000000,
                                            0x200000000000, 0x400000000000, 0x800000000000, 0x1000000000000,
                                            0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000,
                                            0x20000000000000, 0x40000000000000, 0x80000000000000, 0x100000000000000,
                                            0x200000000000000, 0x400000000000000, 0x800000000000000, 0x1000000000000000,
                                            0x2000000000000000, 0x4000000000000000, 0x8000000000000000};
    
    enum access_enum {
        WRITE = 0,
        READ = 1,
        APPEND = 2
    };

    class bitio_stream {
    private:

        FILE *file;
        access_enum mode;
        bool is_inmem = false;
        uint8_t *inmem_cache = nullptr;
        std::string filename;
        unsigned char bit_buffer, *byte_buffer;
        unsigned char bit_count;

        uint64_t buffer_size, byte_index, current_buffer_length;
        bool eof;

        inline void load_buffer();

        inline void load_byte();

        inline void wflush();

        inline void lim_skip(uint8_t n);

        inline void move_to_memory();

    public:

        bitio_stream(std::string filename, access_enum op, bool in_mem = false, uint64_t buffer_size=BITIO_BUFFER_SIZE);

        ~bitio_stream();

        void skip(uint64_t n);

        void seek(int64_t n);

        uint64_t peek(uint8_t n);

        uint64_t read(uint8_t n);

        void write(uint64_t obj, uint64_t n);

        template <typename T>
        void force_write(T *data, uint64_t length) {
            auto byte_data = new uint8_t[length];
            for(uint64_t i = 0; i < length; i++) {
                byte_data[i] = (uint8_t) data[i];
            }
            fwrite(byte_data, 1, length, file);
        }

        void flush();

        void align();

        void force_align();

        void close();

        bool is_eof();

        uint64_t get_file_size();
    };

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
        uint64_t buffer_size, byte_index, current_buffer_length;
        bool eof, read_mode;

        unsigned char wbit_buffer, *wbyte_buffer, wbit_count;
        uint64_t wbyte_index;

        inline void buffer_flush();

        void buffer_t_alloc(struct buffer_t *buf_t, uint64_t buffer_length);

    public:

        bitio_buffer(uint64_t read_buffer_size);

        void write(uint64_t obj, uint64_t n);

        void flush(FILE *file);
    };

    // dedicated byte-dumper.

    class bitio_byte_dumper {
    private:
        uint8_t *data;
        uint64_t size;
        int multiplier;
        char *filename;
        FILE *out;
        char *mode;
    public:
        bitio_byte_dumper(char *filename, bool append = false);

        void write_byte(uint8_t byte);

        uint8_t *get_bytes();

        void dump();
    };
}
