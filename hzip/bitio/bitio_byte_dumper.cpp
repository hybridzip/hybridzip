#include "bitio.h"
#include <cstring>

using namespace bitio;

bitio_byte_dumper::bitio_byte_dumper(char *filename, bool append) {
    data = new uint8_t[HZ_BITIO_BUFFER_SIZE];
    this->filename = filename;
    multiplier = 1;
    size = 0;
    out = nullptr;
    mode = strdup(append ? "a" : "wb");
}

void bitio_byte_dumper::write_byte(uint8_t byte) {
    if (size == multiplier * HZ_BITIO_BUFFER_SIZE) {
        multiplier++;
        data = (uint8_t*)realloc(data, HZ_BITIO_BUFFER_SIZE * multiplier * sizeof(uint8_t));
    }
    data[size++] = byte;
}

void bitio_byte_dumper::dump() {
    out = fopen(filename, mode);
    fwrite(data, 1, size, out);
    fclose(out);
}


uint8_t *bitio_byte_dumper::get_bytes() {
    return data;
}


