#ifndef HYBRIDZIP_HZMTHREAD_H
#define HYBRIDZIP_HZMTHREAD_H

#include <sys/types.h>
#include <cstdint>
#include <thread>
#include <utility>
#include <vector>
#include <hzip/core/entropy/hzrans/hzrans.h>
#include <bitio/bitio.h>
#include <hzip/utils/distribution.h>
#include <hzip/core/blob/hzblob.h>
#include <hzip/core/entropy/hzrans/hzbin.h>
#include <hzip/memory/mem_interface.h>


void hzu_gen_blob(uint64_t alpha, uint16_t scale, uint64_t size, uint64_t *dist,
                  hz_codec_callback callback, std::function<uint64_t(void)> extract, hzrblob_t *targ_blob,
                  hz_cross_encoder cross_encoder = nullptr, bool bypass_normalization = false, hz_memmgr *mgr = nullptr);

void hzu_degen_blob(hzrblob_t blob, uint64_t alpha, uint16_t scale, uint64_t *dist, hz_codec_callback _callback,
                    hz_cross_encoder cross_encoder,
                    bool bypass_normalization = false, std::function<uint64_t()> symbol_callback= nullptr,
                    hz_memmgr *mgr = nullptr);

class hzu_proc: public hz_mem_iface {
private:
    uint nthreads;
    uint64_t alphabet_size;
    uint64_t size;
    uint16_t scale;
    hz_codec_callback callback;
    std::function<uint64_t(void)> *extractors;
    hz_cross_encoder *cross_encoders;
    bool _bypass_normalization;
public:
    hzu_proc(uint n_threads);

    void set_header(uint64_t alphabet_size = 0x100, uint16_t scale = HZRANS_SCALE);

    void set_buffer_size(uint64_t buffer_size);

    void set_callback(hz_codec_callback _callback);

    void set_extractors(std::function<uint64_t(void)> *_extractors);

    void set_cross_encoders(hz_cross_encoder *_cross_encoders);

    void use_only_base_encoder();

    void bypass_normalization();

    hzrblob_set encode();

    std::vector<uint64_t> decode(hzrblob_set set, std::function<uint64_t()> symbol_callback=nullptr);
};


#endif
