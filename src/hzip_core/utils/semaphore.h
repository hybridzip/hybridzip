#ifndef HZIP_UTILS_SEMAPHORE_H
#define HZIP_UTILS_SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore {
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    uint64_t _count;

public:
    Semaphore(uint64_t count = 0) : _count(count) {}

    void acquire();

    void release();
};

#endif
