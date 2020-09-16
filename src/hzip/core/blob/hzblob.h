#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include <malloc.h>
#include <hzip/utils/platform.h>
#include <hzip/core/compressors/compressor_enums.h>
#include <hzip/memory/mem_interface.h>
#include <hzip/utils/common.h>

struct hz_mstate: public hz_mem_iface {
    uint64_t *data;
    uint64_t length;

    hz_mstate() {
        data = nullptr;
        length = 0;
    }

    [[nodiscard]] bool is_empty() const {
        return data == nullptr;
    }
};

struct hz_blob_header: public hz_mem_iface {
    uint8_t *raw;
    uint64_t length;

    hz_blob_header() {
        raw = nullptr;
        length = 0;
    }

    [[nodiscard]] bool is_empty() const {
        return raw == nullptr;
    }
};

struct hzblob_t: public hz_mem_iface {
    hz_blob_header header{};
    hz_mstate *mstate;
    uint32_t *data;
    uint64_t size;
    uint8_t *o_data;
    uint64_t o_size;
    hzcodec::algorithms::ALGORITHM alg;

    hzblob_t() {
        mstate = nullptr;
        data = nullptr;
        o_data = nullptr;
        size = 0;
        o_size = 0;
        alg = hzcodec::algorithms::UNDEFINED;
    }

    void destroy() {
        HZ_FREE(data);
        HZ_FREE(o_data);
        HZ_FREE(header.raw);
    }
};

#endif
