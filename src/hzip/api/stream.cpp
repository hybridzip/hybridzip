#include "stream.h"

uint64_t hz_encode_stream::hzes_b_size(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNDEFINED:
            return 0xffffffffffffffff;
        case hzcodec::algorithms::VICTINI:
            return 0x400000;
    }
}

hz_encode_stream::hz_encode_stream(int _sock, char *_ip_addr, uint16_t _port, hz_processor *_proc) {
    sock = _sock;
    ip_addr = _ip_addr;
    port = _port;
    proc = _proc;
}

/*
 * Protocol:
 * 1. recv blob-algorithm, dest-len, dest
 * 2. infer max blob size from algorithm
 * 3. recv data size
 * 4. recv blob-j
 * 5. encode blob-j
 * 6. cycle hz_processor to avoid overload
 * 7. repeat 4-6 till entire data is received
 * 8. end
 */

void hz_encode_stream::start() {
    uint8_t alg_byte;
    HZ_RECV(&alg_byte, sizeof(alg_byte));

    auto alg = (hzcodec::algorithms::ALGORITHM) alg_byte;

    uint16_t dest_len;
    HZ_RECV(&dest_len, sizeof(dest_len));

    char *dest = rmalloc(char, dest_len);
    HZ_RECV(dest, dest_len);

    uint64_t max_blob_size = hzes_b_size(alg);

    uint64_t data_size;
    HZ_RECV(&data_size, sizeof(data_size));

    while (data_size >= max_blob_size) {
        hzblob_t *blob = rxnew(hzblob_t);
        blob->o_size = max_blob_size;


        data_size -= max_blob_size;
    }
}
