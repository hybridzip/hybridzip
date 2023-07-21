#ifndef HYBRIDZIP_PNG_BUNDLE_H
#define HYBRIDZIP_PNG_BUNDLE_H

#include <png.h>
#include <rainman/rainman.h>
#include <cstdint>
#include <string>
#include <hzip_core/errors/transform.h>

class PNGBundle {
public:
    struct IHDR {
        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
        uint8_t color_type;
        uint8_t interlace_method;
        uint8_t compression_method;
        uint8_t filter_method;
    };

    struct PLTE {
        rainman::ptr<png_color> colors;
    };

    rainman::ptr<uint16_t> buf{};
    uint8_t nchannels{};

    IHDR ihdr{};
    PLTE plte{};

    PNGBundle(
            const rainman::ptr<uint16_t> &buf,
            uint8_t nchannels
    ) : nchannels(nchannels) {
        this->buf = buf;
    }
};

class PNGBundleBuilder : private rainman::Allocator {
private:
    struct MemBuffer {
        rainman::ptr<uint8_t> buffer;
        uint64_t index{};

        void read(uint8_t *dst, uint64_t size, uint64_t n);

        void write(uint8_t *dst, uint64_t size, uint64_t n);
    };

    MemBuffer _mem_buf;

    static void png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length);

    static void png_vector_write_fn(png_structp png_ptr, png_bytep data, png_size_t length);

    static void png_nop_flush_fn(png_structp png_ptr);

    static void png_error_fn(png_structp png_ptr, png_const_charp error_msg);

    static void png_warning_fn(png_structp png_ptr, png_const_charp warning_msg);

    static png_voidp png_malloc_fn(png_structp png_ptr, png_size_t size);

    static void png_free_fn(png_structp png_ptr, png_voidp ptr);

public:
    PNGBundleBuilder() = default;

    explicit PNGBundleBuilder(const rainman::ptr<uint8_t> &buf);

    PNGBundle read_bundle();

    rainman::ptr<uint8_t> write_pixels(const PNGBundle &bundle);
};

#endif
