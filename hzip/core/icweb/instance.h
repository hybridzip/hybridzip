#ifndef HYBRIDZIP_INSANE_CONTEXT_WEB_INSTANCE_H
#define HYBRIDZIP_INSANE_CONTEXT_WEB_INSTANCE_H

#include <vector>
#include "spawn.h"
#include "tracker.h"
#include "../entropy-coders/fgk.h"


namespace icweb {
    class instance {
    private:
        hfc::fgk_tree z0;
        std::vector<icweb::spawn> spawns;

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
