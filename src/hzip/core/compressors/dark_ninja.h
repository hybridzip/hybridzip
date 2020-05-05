#ifndef HYBRIDZIP_DARK_NINJA_H
#define HYBRIDZIP_DARK_NINJA_H

#include <string>
#include <bitio/bitio.h>
#include <hzip/core/entropy/hzrans/hzmthread.h>
#include <hzip/core/blob/hzblobpack.h>
#include <hzip/core/preprocessor/transforms.h>
#include <hzip/utils/boost_utils.h>
#include <hzip/core/models/models.h>

namespace hzcomp_dark_ninja {
    class dark_ninja {
    private:
        bitio::bitio_stream *stream;
    public:
        dark_ninja(std::string filename);

        dark_ninja() {
            // empty-constructor
        };

        void set_file(std::string filename);

        void compress(std::string out_file_name);

        void decompress(std::string out_file_name);
    };
}
#endif
