#ifndef HYBRIDZIP_INSANE_CONTEXT_WEB_CORE_H
#define HYBRIDZIP_INSANE_CONTEXT_WEB_CORE_H

#include <vector>
#include "spawn.h"
#include "../entropy-coders/fgk.h"
#include "tracker.h"


namespace icweb {
    class instance {
    private:
        hfc::fgk_tree z0;
        std::vector<icweb::spawn> spawns;
        std::vector<icweb::tracker> trackers;

        float* noise_buffer;
        HZIP_UINT nbsize;
        float ctx_threshold;
    public:
        instance();
        void set_nbsize(HZIP_UINT size);
        void set_ctx_threshold(float threshold);


    };
}


#endif
