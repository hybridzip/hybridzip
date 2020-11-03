#ifndef HYBRIDZIP_COMPRESSOR_ENUMS_H
#define HYBRIDZIP_COMPRESSOR_ENUMS_H

namespace hzcodec::algorithms {
    enum ALGORITHM {
        UNDEFINED = 0x0,
        VICTINI = 0x1
    };

    inline const char *algorithm_to_str(ALGORITHM alg) {
        switch (alg) {
            case UNDEFINED:
                return "UNDEFINED";
            case VICTINI:
                return "VICTINI";
        }
    }
}

#endif
