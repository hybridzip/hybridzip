#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fstream>
#include <iostream>
#include <stdbool.h>
#include <vector>
#include "../other/platform.h"
#include "../other/constants.h"

#ifndef HZ_BITIO_BUFFER_SIZE
#define HZ_BITIO_BUFFER_SIZE 0x400
#endif

namespace bitio {

    enum access_enum {
        WRITE = 0,
        READ = 1,
        APPEND = 2
    };

    class bitio_stream {
    private:

        FILE *file;
        unsigned char bit_buffer, *byte_buffer;
        unsigned char bit_count;

        uint64_t buffer_size, byte_index, current_buffer_length;
        bool read_mode, eof;

        HZIP_FORCED_INLINE void load_buffer();

        HZIP_FORCED_INLINE void load_byte();

        HZIP_FORCED_INLINE void wflush();


    public:

        bitio_stream(char *filename, access_enum op, uint64_t buffer_size);

        void skip(uint64_t n);

        uint64_t read(uint64_t n);

        void write(uint64_t obj, uint64_t n);

        void flush();

        void align();

        void close();

        bool isEOF();
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

        HZIP_FORCED_INLINE void buffer_flush();

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