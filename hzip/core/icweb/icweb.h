
#ifndef HYBRIDZIP_ICWEB_H
#define HYBRIDZIP_ICWEB_H

#include <vector>
#include "../entropy-coders/fgk.h"

namespace icweb {

    class spawn {
    private:
        HZIP_UINT ref_freq;
        HZIP_SIZE_T total_freq;
        HZIP_SIZE_T* freq;
        HZIP_UINT alphabet_size;
        float* noise_buf;
        hfc::fgk_tree tree;
    public:
        HZIP_UINT lru_index;

        spawn(HZIP_UINT size, float* noise_buf);
        ~spawn();
        void update(HZIP_UINT symbol);
        bin_t encode(HZIP_UINT symbol);
        void merge(spawn* s);
    };

    class instance {
    private:
        hfc::fgk_tree z0;
        std::vector<icweb::spawn> spawns;

        float* noise_buffer;
        HZIP_UINT nbsize;

    public:
        float ctx_threshold;

        instance();
        void set_nbsize(HZIP_UINT size);

    };

    namespace trackers {
        class ctx_tracker {
        private:
            float target;
            float lr;
            float prev_ratio;
            float prev_ctx_threshold;
            float new_ctx_threshold;
            instance* targ_inst;
        public:
            ctx_tracker();

            void track(icweb::instance *inst);

            void feed(HZIP_UINT nbits);
        };
    }
}




#endif
