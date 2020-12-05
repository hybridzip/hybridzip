#ifndef HYBRIDZIP_TYPES_H
#define HYBRIDZIP_TYPES_H

#include <cstdint>
#include <memory>
#include <rainman/rainman.h>

class PNGBundle : public rainman::context {
public:
    uint16_t ***buf{};
    uint32_t width{};
    uint32_t height{};
    uint8_t nchannels{};
    uint8_t depth{};

    PNGBundle(uint16_t ***buf, uint32_t width, uint32_t height, uint8_t nchannels, uint8_t depth) : buf(buf),
                                                                                                    width(width),
                                                                                                    height(height),
                                                                                                    nchannels(nchannels),
                                                                                                    depth(depth) {}


    ~PNGBundle() {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                rfree(buf[i][j]);
            }
            rfree(buf[i]);
        }
        rfree(buf);
    }
};

#endif
