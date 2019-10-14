#ifndef HYBRIDZIP_INSANE_CONTEXT_WEB_TRACKER_H
#define HYBRIDZIP_INSANE_CONTEXT_WEB_TRACKER_H

#include "instance.h"
#include "../../other/platform.h"


namespace icweb {
    class cprs_tracker {
    private:
        float target;
        float avg;
        float* buffer;
    public:
        cprs_tracker();
        void track(icweb::instance* inst);
        void feed(HZIP_UINT nbits);
    };
}

#endif
