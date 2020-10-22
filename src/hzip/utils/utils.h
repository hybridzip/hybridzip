#ifndef HYBRIDZIP_UTILS_H
#define HYBRIDZIP_UTILS_H

#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <iomanip>
#include <ctime>
#include <openssl/sha.h>
#include "common.h"

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


HZ_INLINE std::string hz_sha512(std::string str) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512_CTX sha512;
    SHA512_Init(&sha512);
    SHA512_Update(&sha512, str.c_str(), str.size());
    SHA512_Final(hash, &sha512);
    std::stringstream ss;

    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int) hash[i];
    }
    return ss.str();
}

HZ_INLINE void hz_unary_write(uint64_t n, std::function<void(uint64_t, uint8_t)> writefunc) {
    while (n > 1) {
        writefunc(1, 1);
    }

    writefunc(0, 1);
}

HZ_INLINE uint64_t hz_unary_read(std::function<uint64_t(uint8_t)> readfunc) {
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

#endif
