//
// Created by Vishaal Selvaraj on 04-12-2019.
//

#ifndef HYBRIDZIP_HZRANS_H
#define HYBRIDZIP_HZRANS_H
#ifndef HZRANS_USE_AVX
#define HZRANS_USE_AVX 0
#endif

#ifndef HZRANS_SCALE
#define HZRANS_SCALE 12
#endif

#if HZRANS_USE_AVX
#include "hzrans64avx.h"
#define HYBRIDZIP_HZRANS64_H
#else
#include "hzrans64.h"
#define HYBRIDZIP_HZRANS64AVX_H
#endif

#include "hzrbyte.h"

#endif //HYBRIDZIP_HZRANS_H
