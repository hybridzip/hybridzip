#include "dispatcher.h"

hz_dispatcher::hz_dispatcher() {
    sem_init(&mutex, 0, 1);
}

void hz_dispatcher::register_tag(char *tag, uint64_t mem_peak) {
    sem_wait(&mutex);

    auto mgr = new hz_memmgr;
    mgr->set_peak(mem_peak);
    mgr->set_parent(HZ_MEM_MGR);

    auto str_tag = std::string(tag);

    res_map[str_tag] = mgr;

    sem_post(&mutex);
}

void hz_dispatcher::add_job(hz_codec_job *job) {
    sem_wait(&mutex);

    auto str_tag = std::string(job->tag);

    if (!res_map.contains(str_tag)) {
        auto mgr = new hz_memmgr;

        // default mem-peak per tag is 64MB
        mgr->set_peak(0x4000000);
        mgr->set_parent(HZ_MEM_MGR);

        res_map[str_tag] = mgr;
    }

    jobs.push_back(job);

    sem_post(&mutex);
}

