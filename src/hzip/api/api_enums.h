#ifndef HYBRIDZIP_API_ENUMS_H
#define HYBRIDZIP_API_ENUMS_H

namespace hzapi {
    enum COMMON_CTL {
        COMMON_CTL_SUCCESS = 0x0,
        COMMON_CTL_ERROR = 0xff,
    };

    enum API_CTL {
        API_CTL_STREAM = 0x0,
        API_CTL_QUERY = 0x1,
        API_CTL_HEALTH_CHECK = 0x2,
        API_CTL_CLOSE = 0xff,
    };

    enum STREAM_CTL {
        STREAM_CTL_ENCODE = 0x0,
        STREAM_CTL_DECODE = 0x1,
        STREAM_CTL_WRITE_MSTATE = 0x2,
        STREAM_CTL_READ_MSTATE = 0x3,
    };

    enum ENCODE_CTL {
        ENCODE_CTL_STREAM = 0x0,
        ENCODE_CTL_MSTATE_ADDR = 0x1,
        ENCODE_CTL_ARCHIVE = 0x2,
        ENCODE_CTL_DEST = 0x3,
        ENCODE_CTL_ALGORITHM = 0x4,
        ENCODE_CTL_PIGGYBACK = 0x5,
        ENCODE_CTL_TRAIN = 0x6,
    };

    enum DECODE_CTL {
        DECODE_CTL_STREAM = 0x0,
        DECODE_CTL_MSTATE_ADDR = 0x1,
        DECODE_CTL_ARCHIVE = 0x2,
        DECODE_CTL_SRC = 0x3,
        DECODE_CTL_ALGORITHM = 0x4,
        DECODE_CTL_PIGGYBACK = 0x5,
        DECODE_CTL_MSTATE_STREAM = 0x6,
        DECODE_CTL_BLOB_STREAM = 0x7,
    };

    enum MSTATE_CTL {
        MSTATE_CTL_ARCHIVE = 0x0,
        MSTATE_CTL_ADDR = 0x1,
        MSTATE_CTL_STREAM = 0x2,
        MSTATE_CTL_PIGGYBACK = 0x3
    };

    enum QUERY_CTL {
        QUERY_CTL_ARCHIVE = 0x0,
        QUERY_CTL_CHECK_IF_FILE_EXISTS = 0x1,
        QUERY_CTL_GET_ALL_FILES = 0x2,
        QUERY_CTL_PIGGYBACK = 0x3,
        QUERY_CTL_DELETE_FILE = 0x4,
        QUERY_CTL_DELETE_MSTATE = 0x5,
    };
}

#endif
