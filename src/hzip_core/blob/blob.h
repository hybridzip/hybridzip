#ifndef HYBRIDZIP_BLOB_H
#define HYBRIDZIP_BLOB_H

#include <cstdint>
#include <unordered_map>
#include <rainman/rainman.h>
#include <hzip_core/utils/platform.h>
#include <hzip_codec/compressor_enums.h>
#include <hzip_core/utils/common.h>

struct HZ_MState {
    rainman::ptr<uint8_t> data{};
    hzcodec::algorithms::ALGORITHM alg{};

    HZ_MState();

    [[nodiscard]] uint64_t size() const;

    [[nodiscard]] bool is_empty() const;
};

struct HZ_BlobHeader {
    rainman::ptr<uint8_t> raw;

    HZ_BlobHeader() = default;

    [[nodiscard]] uint64_t size() const;
};

struct HZ_Params {
    std::unordered_map<std::string, std::string> params;

    rainman::ptr<uint8_t> to_byte_array();

    static HZ_Params from_byte_array(const rainman::ptr<uint8_t> &raw);

    template <typename Type=std::string>
    Type get(const std::string &key, const std::string &default_value = "");
};

// status: true indicates that the blob is compressed.

struct HZ_Blob {
    HZ_BlobHeader header{};
    HZ_Params codec_params{};
    rainman::ptr<HZ_MState> mstate;
    rainman::ptr<uint8_t> data;
    uint64_t size{};
    uint64_t o_size{};
    uint64_t mstate_id{};
    bool status{};

    HZ_Blob() = default;

    void evaluate(const rainman::ptr<uint8_t> &original_data);
};

#endif
