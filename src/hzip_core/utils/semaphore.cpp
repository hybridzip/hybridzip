#include "semaphore.h"

void Semaphore::acquire() {
    std::unique_lock<std::mutex> lk(_mutex);
    _cv.wait(lk, [this] { return _count > 0; });
    _count--;
}

void Semaphore::release() {
    std::unique_lock<std::mutex> lk(_mutex);
    _count++;
    _cv.notify_all();
}
