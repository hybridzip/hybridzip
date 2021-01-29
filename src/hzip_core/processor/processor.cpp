#include "processor.h"
#include <hzip_core/errors/processor.h>
#include <hzip_core/core/compressors/compressors.h>

#define HZP_STUB_CALL(f, ...) if (f != nullptr) f(__VA_ARGS__)

HZ_Processor::HZ_Processor(uint64_t n_threads) : _semaphore(n_threads) {
    this->n_threads = n_threads;
}

hzcodec::AbstractCodec *HZ_Processor::hzp_get_codec(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNCOMPRESSED:
            return rnew<hzcodec::Uncompressed>(1);
        case hzcodec::algorithms::VICTINI:
            return rnew<hzcodec::Victini>(1);
        case hzcodec::algorithms::SHARINGAN:
            return rnew<hzcodec::Sharingan>(1);
        default:
            throw ProcessorErrors::InvalidOperationError("Algorithm not found");
    }
}

void HZ_Processor::run(const rainman::ptr<HZ_Job> &job) {
    if (job->type == HZ_Job::CODEC) {
        std::thread([this](const rainman::ptr<HZ_Job> &job) {
            _semaphore.acquire();

            try {
                hzp_run_codec_job(job->codec);
                HZP_STUB_CALL(job->stub->on_success, "Operation completed successfully");
            } catch (std::exception &e) {
                HZP_STUB_CALL(job->stub->on_error, e.what());
            }

            HZP_STUB_CALL(job->stub->on_completed);

            _semaphore.release();

        }, job).detach();
    }
}

void HZ_Processor::hzp_run_codec_job(const rainman::ptr<HZ_CodecJob> &job) {
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

void HZ_Processor::hzp_encode(const rainman::ptr<HZ_CodecJob> &job) {
    if (job->mstate_addr.is_some()) {
        if (job->archive.is_none()) {
            throw ProcessorErrors::InvalidOperationError("Archive is required for mstate-injection by address");
        }

        job->archive.inner()->inject_mstate(job->mstate_addr.inner(), job->blob);
    }

    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    rainman::ptr<HZ_Blob> blob;
    try {
        blob = codec->compress(job->blob);
        blob->evaluate(job->blob->data);

        blob->mstate_id = job->blob->mstate_id;

        if (job->mstate_addr.is_none()) {
            if (job->archive.is_some()) {
                job->archive.inner()->inject_mstate(blob->mstate, blob);
            } else if (job->blob_callback == nullptr) {
                throw ProcessorErrors::InvalidOperationError(
                        "Piggy-back is disabled, null job execution is not allowed");
            }
        }

        if (job->archive.is_some()) {
            auto id = job->archive.inner()->write_blob(blob);
            HZP_STUB_CALL(job->blob_id_callback, id);
        }

        HZP_STUB_CALL(job->blob_callback, blob);
    } catch (std::exception &e) {
        throw ProcessorErrors::GenericError(e.what());
    }
}

void HZ_Processor::hzp_decode(const rainman::ptr<HZ_CodecJob> &job) {
    if (job->archive.is_none() && job->blob_callback == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Piggy-back is disabled, null job execution is not allowed");
    }

    if (job->mstate_addr.is_some() && job->blob->status) {
        if (job->archive.is_none()) {
            throw ProcessorErrors::InvalidOperationError("Archive is required for mstate-injection by address");
        }

        job->archive.inner()->inject_mstate(job->mstate_addr.inner(), job->blob);
    }

    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    rainman::ptr<HZ_Blob> blob;
    try {
        blob = codec->decompress(job->blob);

        HZP_STUB_CALL(job->blob_callback, blob);

        rfree(codec);
    } catch (std::exception &e) {
        rfree(codec);
        throw ProcessorErrors::GenericError(e.what());
    }
}

void HZ_Processor::hzp_train(const rainman::ptr<HZ_CodecJob> &job) {
    if (job->mstate_addr.is_none()) {
        throw ProcessorErrors::InvalidOperationError("Mstate address not found");
    }

    if (job->archive.is_none()) {
        throw ProcessorErrors::InvalidOperationError("Archive not found");
    }

    if (job->archive.inner()->check_mstate_exists(job->mstate_addr.inner())) {
        job->archive.inner()->inject_mstate(job->mstate_addr.inner(), job->blob);
        job->archive.inner()->uninstall_mstate(job->mstate_addr.inner());
    }

    auto codec = hzp_get_codec(job->algorithm);

    if (codec == nullptr) {
        throw ProcessorErrors::InvalidOperationError("Codec not found");
    }

    rainman::ptr<HZ_MState> mstate;

    try {
        mstate = codec->train(job->blob);

        job->archive.inner()->install_mstate(job->mstate_addr.inner(), mstate);

        rfree(codec);
    } catch (std::exception &e) {
        rfree(codec);

        throw ProcessorErrors::GenericError(e.what());
    }

}

// To avoid processor overload.
void HZ_Processor::cycle() {
    _semaphore.acquire();
    _semaphore.release();
}

