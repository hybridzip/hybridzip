#include "bitio.h"

using namespace bitio;


bitio_stream::bitio_stream(char *filename, access_enum op, uint64_t buffer_size) {
    file = nullptr;
    wfile = nullptr;
    if ((op == READ) && !(file = fopen(filename, "rb"))) {
        fprintf(stderr, "File not found");
        return;
    } else if (op == WRITE) {
        wfile = fopen(filename, "wb");
    }
    this->buffer_size = buffer_size;
    byte_buffer = (unsigned char *) malloc(sizeof(unsigned char) * buffer_size);
    wbyte_buffer = (unsigned char *) malloc(sizeof(unsigned char) * buffer_size);
    bit_count = 0;

    bit_buffer = 0;
    current_buffer_length = 0;
    byte_index = 0;
    wbyte_index = 0;
    wbit_buffer = 0;
    wbit_count = 0;
}

void bitio_stream::close() {
    if(file != nullptr) fclose(file);
    if(wfile != nullptr) fclose(wfile);
}

HZIP_FORCED_INLINE void bitio_stream::load_buffer() {
    current_buffer_length = fread(byte_buffer, 1, buffer_size, file);
    eof = current_buffer_length == 0;
    byte_index = 0;
}

HZIP_FORCED_INLINE void bitio_stream::load_byte() {
    if (eof)
        bit_buffer = 0;
    if (byte_index == current_buffer_length)
        load_buffer();
    bit_buffer = byte_buffer[byte_index++];
}

HZIP_FORCED_INLINE void bitio_stream::wflush() {
    fwrite(wbyte_buffer, 1, buffer_size, wfile);
}

uint64_t bitio_stream::read(uint64_t n) {
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

void bitio_stream::skip(uint64_t n) {
    if (bit_count == 0) {
        load_byte();
        bit_count = 8;
    }
    if (bit_count >= n) {
        bit_buffer <<= n;
        bit_count -= n;
    } else {
        uint64_t nbytes = (n - bit_count) >> 3;
        bit_buffer = 0;
        bit_count = 0;
        fseek(file, nbytes, SEEK_CUR);
        load_byte();
        bit_count = 8;
        char rembits = n & bit_masks[3];
        bit_buffer <<= rembits;
        bit_count -= rembits;
    }
}

void bitio_stream::write(uint64_t obj, uint64_t n) {
    uint64_t i = 0;
    obj <<= 0x40 - n;

    unsigned char mask_index = 0;
    while (i++ < n) {
        auto debug_z = (obj & ui64_single_bit_masks[0x3f - mask_index++]);

        wbit_buffer +=  debug_z != 0;
        wbit_count++;
        if (wbit_count == 8) {
            wbyte_buffer[wbyte_index++] = wbit_buffer;
            if (wbyte_index == buffer_size) {
                wflush();
                wbyte_index = 0;
            }
            wbit_buffer = 0;
            wbit_count = 0;
        }
        wbit_buffer <<= 1;
    }
}

void bitio_stream::flush() {
    if (wbit_count == 0) return;
    wbit_buffer <<= 7 - wbit_count;
    wbyte_buffer[wbyte_index++] = wbit_buffer;
    fwrite(wbyte_buffer, 1, wbyte_index, wfile);
}


