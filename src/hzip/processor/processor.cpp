#include "processor.h"
#include <hzip/errors/processor.h>
#include <hzip/core/compressors/compressors.h>

#define HZP_STUB_CALL(f, ...) if (f != nullptr) f(__VA_ARGS__)

HZ_Processor::HZ_Processor(uint64_t n_threads) {
    this->n_threads = n_threads;
    sem_init(&mutex, 0, n_threads);
}

hzcodec::AbstractCodec *HZ_Processor::hzp_get_codec(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNCOMPRESSED:
            return rxnew(hzcodec::Uncompressed);
        case hzcodec::algorithms::VICTINI:
            return rxnew(hzcodec::Victini);
        default:
            throw ProcessorErrors::InvalidOperationError("Algorithm not found");
    }
}

void HZ_Processor::run(HZ_Job *job) {
    if (job == nullptr) {
        return;
    }

    if (job->codec != nullptr) {
        std::thread([this](HZ_Job *job) {
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

void HZ_Processor::hzp_run_codec_job(HZ_CodecJob *job) {
    switch (job->job_type) {
        case HZ_CodecJob::ENCODE: {
            hzp_encode(job);
            break;
        }
        case HZ_CodecJob::DECODE: {
            hzp_decode(job);
            break;
        }
        case HZ_CodecJob::TRAIN: {
            hzp_train(job);
            break;
        }
    }
}

void HZ_Processor::hzp_encode(HZ_CodecJob *job) {
    if (job->use_mstate_addr) {
        if (job->archive == nullptr) {
            throw ProcessorErrors::InvalidOperationError("Archive is required for mstate-injection by address");
        }

        job->archive->inject_mstate(job->mstate_addr, job->blob);
    }

    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    HZ_Blob *blob = nullptr;
    try {
        blob = codec->compress(job->blob);
        blob->evaluate(job->blob->data);

        blob->mstate_id = job->blob->mstate_id;

        if (!job->use_mstate_addr) {
            if (job->archive != nullptr) {
                job->archive->inject_mstate(blob->mstate, blob);
            } else if (job->blob_callback == nullptr) {
                throw ProcessorErrors::InvalidOperationError(
                        "Piggy-back is disabled, null job execution is not allowed");
            }
        }

        if (job->archive != nullptr) {
            auto id = job->archive->write_blob(blob);
            HZP_STUB_CALL(job->blob_id_callback, id);
        }

        HZP_STUB_CALL(job->blob_callback, blob);

        if (blob->mstate != nullptr) {
            blob->mstate->destroy();
        }

        rfree(blob->mstate);
        blob->destroy();
        rfree(blob);
        rfree(codec);

    } catch (std::exception &e) {
        if (blob->mstate != nullptr) {
            blob->mstate->destroy();
        }
        rfree(blob->mstate);
        blob->destroy();
        rfree(blob);

        rfree(codec);

        throw ProcessorErrors::GenericError(e.what());
    }
}

void HZ_Processor::hzp_decode(HZ_CodecJob *job) {
    if (job->archive == nullptr && job->blob_callback == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Piggy-back is disabled, null job execution is not allowed");
    }

    if (job->use_mstate_addr && job->blob->status) {
        if (job->archive == nullptr) {
            throw ProcessorErrors::InvalidOperationError("Archive is required for mstate-injection by address");
        }

        job->archive->inject_mstate(job->mstate_addr, job->blob);
    }

    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    HZ_Blob *blob = nullptr;
    try {
        blob = codec->decompress(job->blob);

        HZP_STUB_CALL(job->blob_callback, blob);

        if (blob->mstate != nullptr) {
            blob->mstate->destroy();
        }
        rfree(blob->mstate);
        blob->destroy();
        rfree(blob);
        rfree(codec);
    } catch (std::exception &e) {
        if (blob->mstate != nullptr) {
            blob->mstate->destroy();
        }
        rfree(blob->mstate);
        blob->destroy();
        rfree(blob);
        rfree(codec);

        throw ProcessorErrors::GenericError(e.what());
    }
}

void HZ_Processor::hzp_train(HZ_CodecJob *job) {
    if (!job->use_mstate_addr) {
        throw ProcessorErrors::InvalidOperationError("Mstate address not found");
    }

    if (job->archive == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Archive not found");
    }

    if (job->archive->check_mstate_exists(job->mstate_addr)) {
        job->archive->inject_mstate(job->mstate_addr, job->blob);
        job->archive->uninstall_mstate(job->mstate_addr);
    }

    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    HZ_MState *mstate = nullptr;

    try {
        mstate = codec->train(job->blob);

        job->archive->install_mstate(job->mstate_addr, mstate);

        mstate->destroy();
        rfree(mstate);
        rfree(codec);
    } catch (std::exception &e) {
        mstate->destroy();
        rfree(mstate);
        rfree(codec);

        throw ProcessorErrors::GenericError(e.what());
    }

}

// To avoid processor overload.
void HZ_Processor::cycle() {
    sem_wait(&mutex);
    sem_post(&mutex);
}

