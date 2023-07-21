#ifndef HYBRIDZIP_CODEC_PROVIDER_H
#define HYBRIDZIP_CODEC_PROVIDER_H

#include <mutex>
#include <unordered_map>
#include "compressor_enums.h"
#include "compressor_base.h"
#include "errors/codec.h"

namespace hzcodec {
    class CodecProvider {
    private:
        static std::unordered_map<algorithms::ALGORITHM, const char *> strmap;
        static std::unordered_map<algorithms::ALGORITHM, AbstractCodec *> codec_map;
        static std::unordered_map<algorithms::ALGORITHM, uint64_t> bsize_map;

    public:
        static inline const char *algorithm_to_str(algorithms::ALGORITHM alg) {
            if (CodecProvider::strmap.contains(alg)) {
                return CodecProvider::strmap[alg];
            } else {
                throw CodecErrors::ResourceResolutionException("Failed to resolve algorithm-string");
            }
        }

        static inline AbstractCodec *algorithm_to_codec(algorithms::ALGORITHM alg) {
            if (CodecProvider::codec_map.contains(alg)) {
                return CodecProvider::codec_map[alg];
            } else {
                throw CodecErrors::ResourceResolutionException("Failed to resolve codec");
            }
        }

        static inline uint64_t algorithm_to_bsize(algorithms::ALGORITHM alg) {
            if (bsize_map.contains(alg)) {
                return bsize_map[alg];
            } else {
                return 0xffffffffffffffff;
            }
        }
    };
}

#endif
