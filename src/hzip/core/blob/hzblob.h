#ifndef HYBRIDZIP_HZBLOB_H
#define HYBRIDZIP_HZBLOB_H

#include <cstdint>
#include <rainman/rainman.h>
#include <hzip/utils/platform.h>
#include <hzip/core/compressors/compressor_enums.h>
#include <hzip/utils/common.h>

struct hz_mstate: public rainman::context {
    uint8_t *data{};
    uint64_t length{};
    hzcodec::algorithms::ALGORITHM alg{};

    hz_mstate() {
        data = nullptr;
        length = 0;
        alg = hzcodec::algorithms::ALGORITHM::UNDEFINED;
    }

    [[nodiscard]] bool is_empty() const {
        return data == nullptr;
    }

    void destroy() {
        rfree(data);
    }
};

struct hz_blob_header: public rainman::context {
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

struct hzblob_t: public rainman::context {
    hz_blob_header header{};
    hz_mstate *mstate;
    uint32_t *data;
    uint64_t size;
    uint8_t *o_data;
    uint64_t o_size;
    uint64_t mstate_id{};

    hzblob_t() {
        mstate = nullptr;
        data = nullptr;
        o_data = nullptr;
        size = 0;
        o_size = 0;
    }

    void destroy() {
        rfree(data);
        rfree(o_data);
        rfree(header.raw);
    }
};

struct hzblob_set {
    hzblob_t *blobs = nullptr;
    uint64_t blob_count{};

    void destroy() const {
        for (uint64_t i = 0; i < blob_count; i++) {
            blobs[i].destroy();
        }
    }
};

#endif
