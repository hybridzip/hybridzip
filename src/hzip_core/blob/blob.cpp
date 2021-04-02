#include <sstream>
#include <bitio/bitio.h>
#include <hzip_core/utils/utils.h>
#include "blob.h"


HZ_MState::HZ_MState() {
    alg = hzcodec::algorithms::UNCOMPRESSED;
}

uint64_t HZ_MState::size() const {
    return data.size();
}

bool HZ_MState::is_empty() const {
    return alg == hzcodec::algorithms::UNCOMPRESSED;
}

uint64_t HZ_BlobHeader::size() const {
    return raw.size();
}

rainman::ptr<uint8_t> HZ_Params::to_byte_array() {
    std::vector<bin_t> bins;

    bins.push_back(bin_t{.obj=params.size(), .n=64});
    for (const auto &param: params) {
        bins.push_back(bin_t{.obj=param.first.length(), .n=64});
        for (const char &c : param.first) {
            bins.push_back(bin_t{.obj=static_cast<uint64_t>(c), .n=8});
        }

        bins.push_back(bin_t{.obj=param.second.length(), .n=64});
        for (const char &c : param.second) {
            bins.push_back(bin_t{.obj=static_cast<uint64_t>(c), .n=8});
        }
    }

    return hz_vecbin_to_raw(bins);
}

HZ_Params HZ_Params::from_byte_array(const rainman::ptr<uint8_t> &raw) {
    auto obj = HZ_Params();
    auto stream = bitio::stream(raw.pointer(), raw.size());

    uint64_t params_size = stream.read(64);
    for (uint64_t i = 0; i < params_size; i++) {
        uint64_t str_len = stream.read(64);
        std::stringstream sstream;

        for (uint64_t j = 0; j < str_len; j++) {
            sstream << static_cast<char>(stream.read(8));
        }

        std::string key = sstream.str();
        str_len = stream.read(64);

        sstream = std::stringstream();
        for (uint64_t j = 0; j < str_len; j++) {
            sstream << static_cast<char>(stream.read(8));
        }

        std::string value = sstream.str();

        obj.params[key] = value;
    }

    return obj;
}

template<>
std::string HZ_Params::get(const std::string &key, const std::string &default_value) {
    if (!params.contains(key)) {
        return default_value;
    }
    return params[key];
}

template<>
uint64_t HZ_Params::get(const std::string &key, const std::string &default_value) {
    if (!params.contains(key)) {
        return std::stoull(default_value);
    }
    return std::stoull(params[key]);
}

template<>
int64_t HZ_Params::get(const std::string &key, const std::string &default_value) {
    if (!params.contains(key)) {
        return std::stoll(default_value);
    }
    return std::stoll(params[key]);
}

void HZ_Blob::evaluate(const rainman::ptr<uint8_t> &original_data) {
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
