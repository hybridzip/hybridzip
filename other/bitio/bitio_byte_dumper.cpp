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
#include <cstring>

using namespace bitio;

bitio_byte_dumper::bitio_byte_dumper(char *filename, bool append) {
    data = new uint8_t[BITIO_BUFFER_SIZE];
    this->filename = filename;
    multiplier = 1;
    size = 0;
    out = nullptr;
    mode = strdup(append ? "a" : "wb");
}

void bitio_byte_dumper::write_byte(uint8_t byte) {
    if (size == multiplier * BITIO_BUFFER_SIZE) {
        multiplier++;
        data = (uint8_t*)realloc(data, BITIO_BUFFER_SIZE * multiplier * sizeof(uint8_t));
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


