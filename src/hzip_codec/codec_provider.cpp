#include "codec_provider.h"
#include "compressors.h"
#include "compressor_enums.h"
#include "compressor_base.h"

using namespace hzcodec;

std::unordered_map<algorithms::ALGORITHM, const char *> CodecProvider::strmap = {
        {algorithms::UNCOMPRESSED,       "UNCOMPRESSED"},
        {algorithms::VICTINI,            "VICTINI"},
        {algorithms::LOSSLESS_SHARINGAN, "LOSSLESS_SHARINGAN"},
};

std::unordered_map<algorithms::ALGORITHM, AbstractCodec *> CodecProvider::codec_map = {
        {algorithms::UNCOMPRESSED,       dynamic_cast<AbstractCodec *>(new Uncompressed())},
        {algorithms::LOSSLESS_SHARINGAN, dynamic_cast<AbstractCodec *>(new LosslessSharingan())},
        {algorithms::VICTINI,            dynamic_cast<AbstractCodec *>(new Victini())},
};

std::unordered_map<algorithms::ALGORITHM, uint64_t> CodecProvider::bsize_map = {
        {algorithms::UNCOMPRESSED,       0x800000},
        {algorithms::VICTINI,            0x400000},
        {algorithms::LOSSLESS_SHARINGAN, 0xffffffffffffffff},
};


