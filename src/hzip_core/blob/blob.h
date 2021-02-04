#ifndef HYBRIDZIP_BLOB_H
#define HYBRIDZIP_BLOB_H

#include <cstdint>
#include <rainman/rainman.h>
#include <hzip_core/utils/platform.h>
#include <hzip_codec/compressor_enums.h>
#include <hzip_core/utils/common.h>

struct HZ_MState {
    rainman::ptr<uint8_t> data{};
    hzcodec::algorithms::ALGORITHM alg{};

    HZ_MState() {
        alg = hzcodec::algorithms::UNCOMPRESSED;
    }

    [[nodiscard]] uint64_t size() const {
        return data.size();
    }

    [[nodiscard]] bool is_empty() const {
        return alg == hzcodec::algorithms::UNCOMPRESSED;
    }
};

struct HZ_BlobHeader{
    rainman::ptr<uint8_t> raw;

    HZ_BlobHeader() = default;

    [[nodiscard]] uint64_t size() const {
        return raw.size();
    }
};

// status: true indicates that the blob is compressed.

struct HZ_Blob {
    HZ_BlobHeader header{};
    rainman::ptr<HZ_MState> mstate;
    rainman::ptr<uint8_t> data;
    uint64_t size{};
    uint64_t o_size{};
    uint64_t mstate_id{};
    bool status{};

    HZ_Blob() = default;

    void evaluate(const rainman::ptr<uint8_t> &original_data) {
        status = size < o_size;
        if (!status) {
            data = original_data;
            size = o_size;

            // Destroy mstate if compression fails.
            // Replace it with a mstate with algorithm = UNCOMPRESSED
            if (!mstate->is_empty()) {
                mstate = rainman::ptr<HZ_MState>();
            }
        }
    }
};

#endif
