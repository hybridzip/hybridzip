#ifndef HYBRIDZIP_INSANE_CONTEXT_WEB_SPAWN_H
#define HYBRIDZIP_INSANE_CONTEXT_WEB_SPAWN_H

#include "../../other/platform.h"
#include "../entropy-coders/fgk.h"

namespace icweb {
    class spawn {
    private:
        HZIP_UINT total_freq;
        hfc::fgk_tree tree;
        HZIP_SIZE_T* freq;
    };

}

#endif
