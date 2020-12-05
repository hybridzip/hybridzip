#ifndef HYBRIDZIP_TYPES_H
#define HYBRIDZIP_TYPES_H

#include <cstdint>
#include <memory>

class Pixar {
public:
    uint8_t ***buf;
    uint32_t width;
    uint32_t height;
    uint8_t nchannels;

    ~Pixar() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                free(buf[i][j]);
            }
            free(buf[i]);
        }
        free(buf);
    }
};

#endif
