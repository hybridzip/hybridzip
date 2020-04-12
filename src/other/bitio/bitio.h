#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include "hzip/utils/platform.h"
#include "hzip/utils/constants.h"

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
        std::string filename;
        unsigned char bit_buffer, *byte_buffer;
        unsigned char bit_count;

        uint64_t buffer_size, byte_index, current_buffer_length;
        bool eof;

        HZIP_FORCED_INLINE void load_buffer();

        HZIP_FORCED_INLINE void load_byte();

        HZIP_FORCED_INLINE void wflush();


    public:

        bitio_stream(std::string filename, access_enum op, uint64_t buffer_size);

        void skip(uint64_t n);

        uint64_t read(uint64_t n);

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
