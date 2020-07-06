#include "hzblobpack.h"

void hz_blob_packer::pack_header(bin_t bin) {
    bin_vec.push_back(bin);
}

void hz_blob_packer::pack(hzrblob_set set) {
    auto bin = unarypx_bin(set.count);
    bin_vec.push_back(bin);

    for (int i = 0; i < set.count; i++) {
        bin = unarypx_bin(set.blobs[i].size);
        bin_vec.push_back(bin);
        bin = unarypx_bin(set.blobs[i].o_size);
        bin_vec.push_back(bin);
    }

    for (int i = 0; i < set.count; i++) {
        auto blob = set.blobs[i];
        for (int j = 0; j < blob.size; j++) {
            bin_vec.push_back(bin_t{.obj=blob.data[j], .n=0x20});
        }
    }
}

void hz_blob_packer::commit(bitio::bitio_stream stream) {
    for (auto &bin : bin_vec) {
        stream.write(bin.obj, bin.n);
    }

    stream.flush();
}

hz_blob_unpacker::hz_blob_unpacker(bitio::bitio_stream *stream) {
    this->stream = stream;
}

HZ_SIZE_T hz_blob_unpacker::unpack_header(unsigned int n) {
    return stream->read(n);
}

hzrblob_set hz_blob_unpacker::unpack() {
    // first retrieve set count.
    auto tmp_stream = this->stream;

    auto lb_stream = [tmp_stream](uint64_t n) {
        uint64_t x = tmp_stream->read(n);
        return x;
    };

    auto set_count = unaryinv_bin(lb_stream).obj;

    hzrblob_set set;
    set.count = set_count;
    set.blobs = new hzrblob_t[set_count];

    for (int i = 0; i < set_count; i++) {
        set.blobs[i].size = unaryinv_bin(lb_stream).obj;
        set.blobs[i].o_size = unaryinv_bin(lb_stream).obj;
    }

    for (int i = 0; i < set_count; i++) {
        set.blobs[i].data = new uint32_t[set.blobs[i].size];
        for (int j = 0; j < set.blobs[i].size; j++) {
            set.blobs[i].data[j] = lb_stream(0x20);
        }
    }

    // align to next-byte.
    stream->align();

    return set;
}
