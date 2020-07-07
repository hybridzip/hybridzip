#include "gtest/gtest.h"
#include <hzip/memory/memmgr.h>
#include <vector>

class Memory : public testing::Test {
    void SetUp() override {

    }
};

TEST(Memory, hzip_memory_unittest_segv) {
    auto memmgr = new hz_memmgr;
    std::vector<int*> ptr_vec;
    int *x = memmgr->hz_malloc<int>(2000);

    for (int i = 0; i < 2000; i++) {
        x[i] = i + 1;
        auto ptr = memmgr->hz_malloc<int>(20);
        ptr_vec.push_back(ptr);
    }

    for (int i = 0; i < ptr_vec.size(); i++) {
        memmgr->hz_free(ptr_vec[i]);
    }

    ASSERT_EQ(memmgr->allocation_size, 2000 * sizeof(int));
    ASSERT_EQ(memmgr->n_allocations, 1);

    memmgr->hz_free(x);
}

