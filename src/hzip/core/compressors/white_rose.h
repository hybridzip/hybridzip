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
    class white_rose : public hz_mem_iface {
    private:
        bitio::bitio_stream *__deprecated_bitio_stream;


    public:
        white_rose(std::string filename);

        white_rose() {
            // empty-constructor
        };

        // deprecated
        void set_file(std::string filename);

        // deprecated
        void compress(std::string out_file_name);

        // deprecated
        void decompress(std::string out_file_name);

        hzblob_t *compress(hzblob_t *blob, hz_mstate *mstate);
    };
}

#endif
