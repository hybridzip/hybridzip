#ifndef HYBRIDZIP_VICTINI_H
#define HYBRIDZIP_VICTINI_H

#include <string>
#include <bitio/bitio.h>
#include <hzip/core/entropy/hzrans/hzmthread.h>
#include <hzip/core/blob/hzblobpack.h>
#include <hzip/core/preprocessor/transforms.h>
#include <hzip/utils/fsutils.h>
#include <hzip/core/models/models.h>
#include <hzip/memory/mem_interface.h>

#include "compressor_base.h"

namespace hzcodec {
    class victini : public hz_abstract_codec, public hz_mem_iface {
    private:
        bitio::bitio_stream *__deprecated_bitio_stream;

        void generate_mstate(hz_mstate *mstate, uint64_t **dict, uint64_t **cdict, int16_t *data, uint64_t length,
                             uint64_t bwt_index);

    public:
        victini(std::string filename);

        victini() {
            // empty-constructor
        };

        // deprecated
        void set_file(std::string filename);

        // deprecated
        void compress(std::string out_file_name);

        // deprecated
        void decompress(std::string out_file_name);

        hzblob_t *compress(hzblob_t *blob) override;

        hzblob_t *decompress(hzblob_t *blob) override;
    };
}

#endif
