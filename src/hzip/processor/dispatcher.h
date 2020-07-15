#ifndef HYBRIDZIP_DISPATCHER_H
#define HYBRIDZIP_DISPATCHER_H

#include <vector>
#include <unordered_map>
#include <semaphore.h>
#include <hzip/memory/mem_interface.h>
#include "job.h"

class hz_dispatcher: public hz_mem_iface {
private:
    std::vector<hz_job*> jobs;
    std::unordered_map<std::string, hz_memmgr*> res_map;
    sem_t mutex;
public:
    hz_dispatcher();

    void register_tag(char *tag, uint64_t mem_peak);

    void add_job(hz_job *job);

    void dispatch();
};

#endif
