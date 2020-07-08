#ifndef HYBRIDZIP_MEM_INTERFACE_H
#define HYBRIDZIP_MEM_INTERFACE_H

#include "memmgr.h"

#define HZ_MALLOC(type, n_elems) this->_hz_memmgr_obj->hz_malloc<type>(n_elems)
#define HZ_FREE(ptr) this->_hz_memmgr_obj->hz_free(ptr)
#define HZ_MEM_INIT(child) child._hz_memmgr_attach_memmgr(this->_hz_memmgr_obj)
#define HZ_MEM_INIT_FROM(mgr, child) child._hz_memmgr_attach_memmgr(mgr)

// Memory interface for using HZ_MALLOC and HZ_FREE more idiomatically.
class hz_mem_iface {
protected:
    hz_memmgr *_hz_memmgr_obj;

public:
    hz_mem_iface() {
        _hz_memmgr_obj = nullptr;
    }

    void _hz_memmgr_attach_memmgr(hz_memmgr *mgr) {
        _hz_memmgr_obj = mgr;
    }
};

#endif
