#include "api.h"

hz_api_instance::hz_api_instance(hz_processor *_processor)  {
    processor = _processor;
}

hz_api *hz_api::limit(uint64_t _max_instances) {
    max_instances = _max_instances;
    return this;
}

hz_api *hz_api::process(uint64_t _n_threads) {
    processor = rxnew(hz_processor, _n_threads);
    return this;
}

hz_api *hz_api::init() {

    return this;
}
