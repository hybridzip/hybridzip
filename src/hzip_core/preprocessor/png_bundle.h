#ifndef HYBRIDZIP_PNG_BUNDLE_H
#define HYBRIDZIP_PNG_BUNDLE_H

#include <png.h>
#include <rainman/rainman.h>
#include <cstdint>
#include <string>
#include <hzip_core/errors/transform.h>

class PNGBundle {
public:
    rainman::ptr<uint16_t> buf{};
    uint32_t width{};
    uint32_t height{};
    uint8_t nchannels{};
    uint8_t depth{};

    PNGBundle(const rainman::ptr<uint16_t> &buf, uint32_t width, uint32_t height, uint8_t nchannels, uint8_t depth) :
            width(width),
            height(height),
            nchannels(nchannels),
            depth(depth) {
        this->buf = buf;
    }
};

class PNGBundleBuilder : private rainman::Allocator {
private:
    FILE *png_file;
public:
    PNGBundleBuilder(uint8_t *buf, uint64_t len) {
        png_file = fmemopen(buf, len, "rb");
    }

    PNGBundleBuilder(std::string filename) {
        png_file = fopen(filename.c_str(), "rb");
    }

    PNGBundle read_pixels() {
        char header[8];    // 8 is the maximum size that can be checked

        /* open file and test for it being a png */
        FILE *fp = png_file;
        if (!fp) {
            throw TransformErrors::InvalidInputError("failed to initialize memory stream");
        }

        fread(header, 1, 8, fp);
        if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8)) {
            throw TransformErrors::InvalidInputError("png header check failed");
        }


        /* initialize stuff */
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

        if (!png_ptr) {
            throw TransformErrors::InvalidOperationError("png_create_read_struct() failed");
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            throw TransformErrors::InvalidOperationError("png_create_info_struct() failed");
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            throw TransformErrors::InvalidOperationError("png_init_io() failed");
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
        png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        png_byte nchannels = png_get_channels(png_ptr, info_ptr);

        int number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        /* read file */
        if (setjmp(png_jmpbuf(png_ptr))) {
            throw TransformErrors::InvalidOperationError("png_read_image() failed");
        }

        auto row_pointers = rmalloc<png_bytep>(height);
        for (int y = 0; y < height; y++) {
            row_pointers[y] = rmalloc<png_byte>(png_get_rowbytes(png_ptr, info_ptr));
        }

        png_read_image(png_ptr, row_pointers);
        fclose(fp);

        auto pixar = rainman::ptr<uint16_t>(nchannels * height * width);
        uint64_t sz = height * width;
        uint64_t sy = width;

        for (uint64_t y = 0, ly = 0; y < height; y++, ly += sy) {
            for (uint64_t x = 0; x < width; x++) {
                for (uint64_t z = 0, lz = 0; z < nchannels; z++, lz += sz) {
                    pixar[lz + ly + x] = row_pointers[y][nchannels * x + z];
                }
            }
        }

        for (int y = 0; y < height; y++) {
            rfree(row_pointers[y]);
        }

        rfree(row_pointers);


        return PNGBundle(
                pixar,
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                nchannels,
                bit_depth
        );
    }
};

#endif
