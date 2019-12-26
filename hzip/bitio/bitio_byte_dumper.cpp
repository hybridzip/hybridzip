#include "bitio.h"

using namespace bitio;

bitio_byte_dumper::bitio_byte_dumper(char *filename) {
    this->filename = filename;
}

void bitio_byte_dumper::write_byte(uint8_t byte) {
    data.push_back(byte);
}

void bitio_byte_dumper::dump() {
    out = fopen(filename, "wb");
    uint8_t *bytes = new uint8_t[data.size()];
    for(int i = 0; i < data.size(); i++) {
        bytes[i] = data[i];
    }
    fwrite(bytes, 1, data.size(), out);
    fclose(out);
}


uint8_t *bitio_byte_dumper::get_bytes() {
    uint8_t *bytes = new uint8_t[data.size()];
    for(int i = 0; i < data.size(); i++) {
        bytes[i] = data[i];
    }
    return bytes;
}


