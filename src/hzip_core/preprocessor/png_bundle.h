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
        int bit_depth;
        int color_type;
        int interlace_method;
        int compression_method;
        int filter_method;
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

        void read(uint8_t *dst, uint64_t size, uint64_t n) {
            for (uint64_t i = 0; i < size * n; i++) {
                dst[i] = buffer[index++];
            }
        }

        void write(uint8_t *dst, uint64_t size, uint64_t n) {
            for (uint64_t i = 0; i < size * n; i++) {
                buffer[index++] = dst[i];
            }
        }
    };

    MemBuffer _mem_buf;

    static void png_read_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
        auto mem_buf = static_cast<MemBuffer *>(png_get_io_ptr(png_ptr));
        mem_buf->read(data, 1, length);
    }

    static void png_vector_write_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
        auto vec = static_cast<std::vector<uint8_t> *>(png_get_io_ptr(png_ptr));

        for (uint64_t i = 0; i < length; i++) {
            vec->push_back(data[i]);
        }
    }

    static void png_nop_flush_fn(png_structp png_ptr) {

    }

    static void png_error_fn(png_structp png_ptr, png_const_charp error_msg) {
        throw TransformErrors::InvalidOperationError(std::string("libpng: ") + error_msg);
    }

    static void png_warning_fn(png_structp png_ptr, png_const_charp warning_msg) {
        LOG_F(WARNING, "libpng: %s", warning_msg);
    }

    static png_voidp png_malloc_fn(png_structp png_ptr, png_size_t size) {
        return rainman::Allocator().rmalloc<uint8_t>(size);
    }

    static void png_free_fn(png_structp png_ptr, png_voidp ptr) {
        rainman::Allocator().rfree(static_cast<uint8_t *>(ptr));
    }

public:
    PNGBundleBuilder() = default;

    explicit PNGBundleBuilder(const rainman::ptr<uint8_t> &buf) {
        _mem_buf.buffer = buf;
    }

    PNGBundle read_pixels() {
        char header[8];

        _mem_buf.read(reinterpret_cast<uint8_t *>(header), 1, 8);
        if (png_sig_cmp(reinterpret_cast<png_bytep>(header), 0, 8)) {
            throw TransformErrors::InvalidInputError("png header check failed");
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

        if (!png_ptr) {
            throw TransformErrors::InvalidOperationError("png_create_read_struct() failed");
        }

        png_set_mem_fn(png_ptr, nullptr, png_malloc_fn, png_free_fn);

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            throw TransformErrors::InvalidOperationError("png_create_info_struct() failed");
        }

        png_set_error_fn(png_ptr, nullptr, png_error_fn, png_warning_fn);
        png_set_read_fn(png_ptr, &_mem_buf, png_read_fn);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
        png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        png_byte nchannels = png_get_channels(png_ptr, info_ptr);

        int number_of_passes = png_set_interlace_handling(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        uint64_t buffer_size = 0;
        auto row_pointers = rmalloc<png_bytep>(height);
        for (int y = 0; y < height; y++) {
            uint64_t row_byte_count = png_get_rowbytes(png_ptr, info_ptr);
            buffer_size += row_byte_count;
            row_pointers[y] = rmalloc<png_byte>(row_byte_count);
        }

        png_read_image(png_ptr, row_pointers);

        auto pixar = rainman::ptr<uint16_t>(buffer_size);
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

        auto bundle = PNGBundle(
                pixar,
                nchannels
        );

        bundle.ihdr.width = width;
        bundle.ihdr.height = height;
        bundle.ihdr.bit_depth = bit_depth;
        bundle.ihdr.color_type = color_type;
        bundle.ihdr.compression_method = png_get_compression_type(png_ptr, info_ptr);
        bundle.ihdr.filter_method = png_get_filter_type(png_ptr, info_ptr);
        bundle.ihdr.interlace_method = png_get_interlace_type(png_ptr, info_ptr);

        // Store PLTE in bundle if necessary
        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_colorp color_ptr;
            int num_colors = 0;
            png_get_PLTE(png_ptr, info_ptr, &color_ptr, &num_colors);

            auto colors = rainman::ptr<png_color>(num_colors);
            for (int i = 0; i < num_colors; i++) {
                colors[i] = color_ptr[i];
            }

            bundle.plte.colors = colors;
        }

        png_destroy_info_struct(png_ptr, &info_ptr);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);

        return bundle;
    }

    rainman::ptr<uint8_t> write_pixels(const PNGBundle &bundle) {
        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

        if (!png_ptr) {
            throw TransformErrors::InvalidOperationError("png_create_write_struct() failed");
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            throw TransformErrors::InvalidOperationError("png_create_info_struct() failed");
        }


    }
};

#endif
