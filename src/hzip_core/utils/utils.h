#ifndef HYBRIDZIP_UTILS_H
#define HYBRIDZIP_UTILS_H

#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/sha.h>
#include <hzip_core/errors/utils.h>
#include "common.h"
#include "platform.h"

HZ_INLINE uint64_t hz_u64log2(uint64_t n) {
    if (n == 0) return 0;
    uint64_t count = 0;
    while (n > 0) {
        n >>= 1;
        count++;
    }
    return --count;
}

HZ_INLINE bin_t hz_elias_gamma(uint64_t n) {
    if (n == 0) return bin_t{.obj=0, .n=1};
    auto lg2 = hz_u64log2(n);
    auto lg2_copy = lg2;
    uint64_t num = 1;

    while (lg2_copy) {
        num <<= 1;
        num++;
        lg2_copy--;
    }

    num <<= 2; // add 0 at the end for unary.
    num <<= lg2; // lshift by logn bits
    num += n; // add n as suffix
    return bin_t{.obj=num, .n=(HZ_UINT) (1 + ((1 + lg2) << 1))};
}

HZ_INLINE bin_t hz_elias_gamma_inv(std::function<uint64_t(uint64_t)> readfunc) {
    uint64_t count = 0;

    while (readfunc(1) != 0) {
        count++;
    }

    uint64_t obj = readfunc(count);
    return bin_t{.obj=obj, .n=(HZ_UINT) count};
}


HZ_INLINE std::string hz_sha512(const std::string& str) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512_CTX sha512;
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, str.c_str(), str.size());
    SHA512_Final(hash, &sha512);
    std::stringstream ss;

    for (unsigned char i : hash) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) i;
    }
    return ss.str();
}

HZ_INLINE void hz_unary_write(uint64_t n, const std::function<void(uint64_t, uint8_t)>& writefunc) {
    while (n > 1) {
        writefunc(1, 1);
    }

    writefunc(0, 1);
}

HZ_INLINE uint64_t hz_unary_read(const std::function<uint64_t(uint8_t)>& readfunc) {
    uint64_t n = 0;
    while (readfunc(1)) n++;
    return n + 1;
}

HZ_INLINE uint64_t hz_rand64() {
    std::random_device rd;
    static thread_local auto _utils_mersenne_twister_engine_rand64 = std::mt19937_64(rd());

    std::uniform_int_distribution<uint64_t> distribution(0, 0xffffffffffffffff);
    return distribution(_utils_mersenne_twister_engine_rand64);
}

HZ_INLINE uint64_t hz_enc_token(std::string pwd, uint64_t x) {
    char *y = (char *) &x;

    for (int i = 0; i < pwd.length(); i++) {
        y[i & 0x7] ^= pwd[i];
    }

    return x;
}

HZ_INLINE void hz_u64_to_u8buf(uint64_t x, const rainman::ptr<uint8_t> &buf) {
    for (int i = 0; i < 8; i++) {
        buf[i] = x & 0xff;
        x >>= 0x8;
    }
}

HZ_INLINE uint64_t hz_u8buf_to_u64(const rainman::ptr<uint8_t> &buf) {
    uint64_t x = 0;
    for (int i = 7; i >= 0; i--) {
        x <<= 0x8;
        x += buf[i];
    }

    return x;
}

HZ_INLINE rainman::ptr<uint32_t> u8_to_u32ptr(const rainman::ptr<uint8_t> &arr, uint64_t n) {
    uint64_t n32 = (n >> 2) + (n & 0x3 ? 1 : 0);

    auto arr32 = rainman::ptr<uint32_t>(n32);

    uint32_t tmp = 0;
    uint64_t j = 0;

    for (uint64_t i = 0; i < n; i++) {
        tmp <<= 0x8;
        tmp += arr[i];

        if (((i + 1) & 3) == 0) {
            arr32[j++] = tmp;
            tmp = 0;
        }
    }

    return arr32;
}

HZ_INLINE rainman::ptr<uint8_t> u32_to_u8ptr(const rainman::ptr<uint32_t> &arr, uint64_t n) {
    auto arr8 = rainman::ptr<uint8_t>(n << 2);

    for (uint64_t i = 0, j = 0; i < n; i++, j += 4) {
        uint32_t x = arr[i];

        arr8[j + 3] = x & 0xff;
        x >>= 8;
        arr8[j + 2] = x & 0xff;
        x >>= 8;
        arr8[j + 1] = x & 0xff;
        x >>= 8;
        arr8[j] = x & 0xff;
    }

    return arr8;
}

inline void hz_assert(bool exp, const std::string &msg = "Assertion failed") {
    if (!exp) {
        throw UtilErrors::InternalError(msg);
    }
}

#define HZ_ASSERT(exp, msg) hz_assert(exp, msg)

#endif
