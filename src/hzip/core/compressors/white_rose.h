#ifndef HYBRIDZIP_WHITE_ROSE_H
#define HYBRIDZIP_WHITE_ROSE_H

#include <string>
#include <bitio/bitio.h>
#include <hzip/core/entropy/hzrans/hzmthread.h>
#include <hzip/core/blob/hzblobpack.h>
#include <hzip/core/preprocessor/transforms.h>
#include <hzip/utils/fsutils.h>
#include <hzip/core/models/models.h>
#include <hzip/memory/mem_interface.h>

namespace hzcodec {
    class white_rose: public hz_mem_iface {
    private:
        bitio::bitio_stream *stream;


    public:
        white_rose(std::string filename);

        white_rose() {
            // empty-constructor
        };

        void set_file(std::string filename);

        void compress(std::string out_file_name);

        void decompress(std::string out_file_name);

        static hzblob_t compress(hzblob_t *blob, hz_ptable *ptable = nullptr);


    };
}

#endif
