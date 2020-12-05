#ifndef HYBRIDZIP_PNG_BUNDLE_H
#define HYBRIDZIP_PNG_BUNDLE_H

#include <png.h>
#include <rainman/rainman.h>
#include <cstdint>
#include <string>
#include "types.h"


class PNGBundleBuilder : public rainman::context {
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
        if (!fp)
            fprintf(stderr, ("[read_png_file] File could not be opened for reading"));
        fread(header, 1, 8, fp);
        if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8))
            fprintf(stderr, ("[read_png_file] File could not be opened for reading"));


        /* initialize stuff */
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

        if (!png_ptr)
            fprintf(stderr, "[read_png_file] png_create_read_struct failed");

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
            fprintf(stderr, "[read_png_file] png_create_info_struct failed");

        if (setjmp(png_jmpbuf(png_ptr)))
            fprintf(stderr, "[read_png_file] Error during init_io");

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
        if (setjmp(png_jmpbuf(png_ptr)))
            fprintf(stderr, "[read_png_file] Error during read_image");

        auto row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
        for (int y = 0; y < height; y++)
            row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr, info_ptr));

        png_read_image(png_ptr, row_pointers);
        fclose(fp);

        auto ***pixar = rmalloc(uint16_t **, height);
        for (int y = 0; y < height; y++) {
            pixar[y] = rmalloc(uint16_t *, width);
            for (int x = 0; x < width; x++) {
                pixar[y][x] = rmalloc(uint16_t, nchannels);
                for (int z = 0; z < nchannels; z++) {
                    pixar[y][x][z] = row_pointers[y][3 * x + z];
                }
            }
        }

        for (int y = 0; y < height; y++) {
            rfree(row_pointers[y]);
        }

        rfree(row_pointers);

        return ronew(PNGBundle, pixar, static_cast<uint32_t>(width), static_cast<uint32_t>(height), nchannels,
                     bit_depth);
    }
};

#endif
