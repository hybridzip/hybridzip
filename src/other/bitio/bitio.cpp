#include "bitio.h"
#include <hzip/utils/boost_utils.h>

using namespace bitio;

bitio_stream::bitio_stream(std::string filename, access_enum op, uint64_t buffer_size) {
    file = nullptr;
    this->filename = filename;

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

}


void bitio_stream::close() {
    free(byte_buffer);
    if (file != nullptr) fclose(file);
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
    fwrite(byte_buffer, 1, buffer_size, file);
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
    for (int i = 0; i < n; i++) {
        read(1);
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
    read(bit_count);
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
    return hzboost::get_file_size(filename);
}




