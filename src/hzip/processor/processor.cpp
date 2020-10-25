#include "processor.h"
#include <hzip/errors/processor.h>
#include <hzip/core/compressors/compressors.h>

#define HZP_STUB_CALL(f, ...) if (f != nullptr) f(__VA_ARGS__)

hz_processor::hz_processor(uint64_t n_threads) {
    this->n_threads = n_threads;
    sem_init(&mutex, 0, n_threads);
}

hzcodec::abstract_codec *hz_processor::hzp_get_codec(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNDEFINED:
            return nullptr;
        case hzcodec::algorithms::VICTINI:
            return rxnew(hzcodec::victini);
        default:
            throw ProcessorErrors::InvalidOperationError("Algorithm not found");
    }
}

void hz_processor::run(hz_job *job) {
    if (job == nullptr) {
        return;
    }

    if (job->codec != nullptr) {
        std::thread([this](hz_job *job) {
            sem_wait(&mutex);

            try {
                this->hzp_run_codec_job(job->codec);
                HZP_STUB_CALL(job->stub->on_success, "Operation completed successfully");
            } catch (std::exception &e) {
                HZP_STUB_CALL(job->stub->on_error, e.what());
            }

            HZP_STUB_CALL(job->stub->on_completed);

            sem_post(&mutex);

        }, job).detach();
    }
}

void hz_processor::hzp_encode(hz_codec_job *job) {
    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    if (job->use_mstate_addr) {
        if (job->archive == nullptr) {
            throw ProcessorErrors::InvalidOperationError("Archive is required for mstate-injection by address");
        }

        job->archive->inject_mstate(job->mstate_addr, job->blob);
    }

    auto *blob = codec->compress(job->blob);

    if (!job->use_mstate_addr) {
        if (job->archive != nullptr) {
            job->archive->inject_mstate(blob->mstate, blob);
        } else if (job->blob_callback == nullptr) {
            throw ProcessorErrors::InvalidOperationError("Piggy-back is disabled, null job execution is not allowed");
        }
    }

    if (job->archive != nullptr) {
        auto id = job->archive->write_blob(blob);
        HZP_STUB_CALL(job->blob_id_callback, id);
    }

    HZP_STUB_CALL(job->blob_callback, blob);

    blob->destroy();
    rfree(blob);
    rfree(codec);
}

void hz_processor::hzp_run_codec_job(hz_codec_job *job) {
    switch (job->job_type) {
        case hz_codec_job::ENCODE: {
            hzp_encode(job);
            break;
        }
        case hz_codec_job::DECODE: {
            hzp_decode(job);
            break;
        }
        case hz_codec_job::TRAIN: {
            hzp_train(job);
            break;
        }
    }
}

void hz_processor::hzp_decode(hz_codec_job *job) {
    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    if (job->archive == nullptr && job->blob_callback == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Piggy-back is disabled, null job execution is not allowed");
    }

    if (job->use_mstate_addr) {
        if (job->archive == nullptr) {
            throw ProcessorErrors::InvalidOperationError("Archive is required for mstate-injection by address");
        }

        job->archive->inject_mstate(job->mstate_addr, job->blob);
    }

    if (job->blob->mstate == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Missing mstate");
    }

    auto *blob = codec->decompress(job->blob);

    HZP_STUB_CALL(job->blob_callback, blob);
    blob->destroy();
}


// To avoid processor overload.
void hz_processor::cycle() {
    sem_wait(&mutex);
    sem_post(&mutex);
}

void hz_processor::hzp_train(hz_codec_job *job) {
    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    if (!job->use_mstate_addr) {
        throw ProcessorErrors::InvalidOperationError("Mstate address not found");
    }

    if (job->archive == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Archive not found");
    }

    auto *mstate = codec->train(job->blob);
    job->archive->install_mstate(job->mstate_addr, mstate);

    mstate->destroy();
    rfree(codec);
}

