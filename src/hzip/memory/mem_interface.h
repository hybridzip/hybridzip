#ifndef HYBRIDZIP_MEM_INTERFACE_H
#define HYBRIDZIP_MEM_INTERFACE_H

#include "memmgr.h"

// Memory interface for using HZ_MALLOC and HZ_FREE more idiomatically.
class hz_mem_iface {
protected:
    hz_memmgr *__hz_memmgr_obj;

public:
    hz_mem_iface() {
        __hz_memmgr_obj = nullptr;
    }

    void attach_memmgr(hz_memmgr *mgr) {
        __hz_memmgr_obj = mgr;
    }
};

#endif
