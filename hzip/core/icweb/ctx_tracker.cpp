#include "icweb.h"

using namespace icweb::trackers;

ctx_tracker::ctx_tracker() {
    lr = 0.5;
    prev_ratio = 0;
    prev_ctx_threshold = 0;
    new_ctx_threshold = 0;
    target = 8;
    targ_inst = nullptr;
}

void ctx_tracker::track(icweb::instance *inst) {
    targ_inst = inst;
    new_ctx_threshold = inst->ctx_threshold;
}

void ctx_tracker::feed(HZIP_UINT nbits) {
    float nratio = 8 / (float)nbits;
    float dy = nratio - prev_ratio;
    float dx = new_ctx_threshold - prev_ctx_threshold;
    prev_ctx_threshold = new_ctx_threshold;
    new_ctx_threshold += (dy/dx) * lr;
    targ_inst->ctx_threshold = new_ctx_threshold;
}

