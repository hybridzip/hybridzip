#include "streamer.h"
#include <hzip/api/providers/archive_provider.h>
#include <hzip/utils/validation.h>
#include <hzip/api/api_enums.h>
#include <hzip/errors/api.h>

using namespace hzapi;

#define HZ_MIN(a, b) (a) < (b) ? (a) : (b)

uint64_t Streamer::hzes_b_size(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNCOMPRESSED:
            return 0x800000;
        case hzcodec::algorithms::VICTINI:
            return 0x400000;
        default:
            return 0xffffffffffffffff;
    }
}

Streamer::Streamer(int _sock, char *_ip_addr, uint16_t _port, HZ_Processor *_proc,
                   hzapi::ArchiveProvider *_archive_provider) {
    sock = _sock;
    ip_addr = _ip_addr;
    port = _port;
    processor = _proc;
    archive_provider = _archive_provider;
    sem_init(&mutex, 0, 1);
}

void Streamer::encode() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint16_t dest_len{};
    uint64_t data_len{};
    uint8_t word{};
    uint64_t algorithm{};

    char *archive_path{};
    char *mstate_addr{};
    char *dest{};

    bool piggy_back{};

    HZ_Archive *archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((ENCODE_CTL) word) {
            case ENCODE_CTL_STREAM: {
                if (!algorithm) {
                    throw ApiErrors::InvalidOperationError("No algorithm was provided");
                }

                if (archive != nullptr && dest != nullptr) {
                    if (archive->check_file_exists(dest)) {
                        throw ApiErrors::InvalidOperationError("File already exists");
                    }
                }

                if (!piggy_back && dest == nullptr) {
                    throw ApiErrors::InvalidOperationError("Null operations are not allowed");
                }

                uint64_t max_blob_size = hzes_b_size((hzcodec::algorithms::ALGORITHM) algorithm);

                HZ_RECV(&data_len, sizeof(data_len));

                std::vector<uint64_t> blob_ids;

                // Start raw-streaming using manual sync.
                HZ_RECV_SYNC;

                while (data_len > 0) {
                    sem_wait(&mutex);

                    // Avoid processor overload.
                    processor->cycle();

                    auto *blob = rxnew(HZ_Blob);

                    max_blob_size = HZ_MIN(max_blob_size, data_len);

                    HZAPI_LOGF(INFO, "(%s) Compressing blob of size: %lu bytes", hzcodec::algorithms::algorithm_to_str(
                            static_cast<hzcodec::algorithms::ALGORITHM>(algorithm)), max_blob_size);

                    blob->o_size = max_blob_size;
                    blob->data = rmalloc(uint8_t, max_blob_size);

                    // Enable raw streaming in hzip-protocol
                    t_recv(blob->data, max_blob_size, false);

                    // Construct hz_job struct for processing.
                    auto *job = rnew(HZ_Job);
                    job->codec = rnew(HZ_CodecJob);

                    job->codec->algorithm = (hzcodec::algorithms::ALGORITHM) algorithm;
                    job->codec->archive = archive;
                    job->codec->use_mstate_addr = mstate_addr != nullptr;
                    if (mstate_addr != nullptr) {
                        job->codec->mstate_addr = mstate_addr;
                    }
                    job->codec->blob = blob;

                    if (archive != nullptr) {
                        job->codec->blob_id_callback = [&blob_ids](uint64_t id) {
                            blob_ids.push_back(id);
                        };
                    }

                    if (piggy_back) {
                        job->codec->blob_callback = [this](HZ_Blob *cblob) {
                            uint64_t alg = cblob->mstate->alg;
                            HZ_SEND(&alg, sizeof(alg));
                            HZ_SEND(&cblob->mstate->length, sizeof(cblob->mstate->length));
                            HZ_SEND(cblob->mstate->data, cblob->mstate->length);

                            HZ_SEND(&cblob->header.length, sizeof(cblob->header.length));
                            HZ_SEND(cblob->header.raw, cblob->header.length);

                            HZ_SEND(&cblob->size, sizeof(cblob->size));
                            HZ_SEND(cblob->data, sizeof(uint32_t) * cblob->size);
                        };
                    }

                    job->codec->job_type = HZ_CodecJob::JOBTYPE::ENCODE;

                    job->stub = rnew(HZ_JobStub);
                    job->stub->on_completed = [this, job]() {
                        job->codec->blob->destroy();
                        rfree(job->codec->blob);
                        rfree(job->codec);
                        rfree(job->stub);
                        rfree(job);
                        sem_post(&mutex);
                    };

                    job->stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    // Dispatch hz_job to hz_processor.
                    processor->run(job);
                    data_len -= max_blob_size;
                }


                sem_wait(&mutex);

                if (archive != nullptr && dest != nullptr) {
                    // Allocate outside the scope of the rainman module for persistence.
                    auto *blob_id_arr = rmemmgr->get_parent()->r_malloc<uint64_t>(blob_ids.size());
                    for (int i = 0; i < blob_ids.size(); i++) {
                        blob_id_arr[i] = blob_ids[i];
                    }

                    HZ_ArchiveFile file{};
                    file.blob_ids = blob_id_arr;
                    file.blob_count = blob_ids.size();

                    archive->create_file_entry(dest, file);
                }

                sem_post(&mutex);

                success("Operation completed successfully");

                return;
            }
            case ENCODE_CTL_MSTATE_ADDR: {
                if (mstate_addr != nullptr) {
                    rfree(mstate_addr);
                }

                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rmalloc(char, mstate_addr_len + 1);
                mstate_addr[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr, mstate_addr_len);
                break;
            }
            case ENCODE_CTL_ARCHIVE: {
                if (archive_path != nullptr) {
                    rfree(archive_path);
                }

                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rmalloc(char, archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path, archive_path_len);

                archive = archive_provider->provide(archive_path);
                break;
            }
            case ENCODE_CTL_DEST: {
                if (dest != nullptr) {
                    rfree(dest);
                }

                HZ_RECV(&dest_len, sizeof(dest_len));

                dest = rmalloc(char, dest_len + 1);
                dest[dest_len] = 0;

                HZ_RECV(dest, dest_len);

                hz_validate_path(dest);
                break;
            }
            case ENCODE_CTL_ALGORITHM: {
                HZ_RECV(&algorithm, sizeof(algorithm));
                break;
            }
            case ENCODE_CTL_PIGGYBACK: {
                piggy_back = true;
                break;
            }
            case ENCODE_CTL_TRAIN: {
                if (!algorithm) {
                    throw ApiErrors::InvalidOperationError("No algorithm was provided");
                }

                if (mstate_addr == nullptr) {
                    throw ApiErrors::InvalidOperationError("Mstate address not found");
                }

                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive not found");
                }

                HZ_RECV(&data_len, sizeof(data_len));

                uint64_t max_blob_size = hzes_b_size((hzcodec::algorithms::ALGORITHM) algorithm);

                // Start raw-streaming using manual sync.
                HZ_RECV_SYNC;

                uint64_t batch_count = 0;

                while (data_len > 0) {
                    sem_wait(&mutex);
                    HZAPI_LOGF(INFO, "(%s) Training: '%s' - Batch: %lu", hzcodec::algorithms::algorithm_to_str(
                            static_cast<hzcodec::algorithms::ALGORITHM>(algorithm)), mstate_addr, ++batch_count);

                    // Avoid processor overload.
                    processor->cycle();

                    auto *blob = rxnew(HZ_Blob);

                    max_blob_size = HZ_MIN(max_blob_size, data_len);

                    blob->o_size = max_blob_size;
                    blob->data = rmalloc(uint8_t, max_blob_size);

                    // Enable raw streaming in hzip-protocol
                    t_recv(blob->data, max_blob_size, false);

                    // Construct hz_job struct for processing.
                    auto *job = rnew(HZ_Job);
                    job->codec = rnew(HZ_CodecJob);

                    job->codec->algorithm = (hzcodec::algorithms::ALGORITHM) algorithm;
                    job->codec->archive = archive;
                    job->codec->use_mstate_addr = mstate_addr != nullptr;
                    if (mstate_addr != nullptr) {
                        job->codec->mstate_addr = mstate_addr;
                    }
                    job->codec->blob = blob;

                    job->codec->job_type = HZ_CodecJob::JOBTYPE::TRAIN;

                    job->stub = rnew(HZ_JobStub);
                    job->stub->on_completed = [this, job]() {
                        job->codec->blob->destroy();
                        rfree(job->codec->blob);
                        rfree(job->codec);
                        rfree(job->stub);
                        rfree(job);
                        sem_post(&mutex);
                    };

                    job->stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    // Dispatch hz_job to hz_processor.
                    processor->run(job);

                    data_len -= max_blob_size;
                }

                sem_wait(&mutex);
                sem_post(&mutex);

                success("Operation completed successfully");
                return;
            }
            default: {
                error("Invalid command");
            }
        }
    }
}

void Streamer::decode() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint16_t src_len{};
    uint8_t word{};
    uint64_t algorithm{};

    HZ_MState *mstate{};
    HZ_Blob *blob{};

    char *archive_path{};
    char *mstate_addr{};
    char *src{};

    bool piggy_back{};

    HZ_Archive *archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((DECODE_CTL) word) {
            case DECODE_CTL_STREAM: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggy-back was disabled");
                }

                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (src == nullptr) {
                    throw ApiErrors::InvalidOperationError("Source not provided");
                }

                sem_wait(&mutex);

                auto file_entry = archive->read_file_entry(src);

                HZ_SEND(&file_entry.blob_count, sizeof(file_entry.blob_count));

                for (uint64_t i = 0; i < file_entry.blob_count; i++) {
                    auto *src_blob = archive->read_blob(file_entry.blob_ids[i]);

                    // Construct hz_job

                    auto *job = rnew(HZ_Job);
                    job->codec = rnew(HZ_CodecJob);
                    job->stub = rnew(HZ_JobStub);

                    job->codec->archive = archive;
                    job->codec->job_type = HZ_CodecJob::JOBTYPE::DECODE;
                    job->codec->blob = src_blob;
                    job->codec->use_mstate_addr = false;
                    job->codec->algorithm = src_blob->mstate->alg;

                    job->codec->blob_callback = [this](HZ_Blob *dblob) {
                        HZ_SEND(&dblob->o_size, sizeof(dblob->o_size));
                        HZ_SEND(dblob->data, dblob->o_size);
                    };

                    job->stub->on_completed = [this, job]() {
                        job->codec->blob->destroy();
                        rparentmgr->r_free(job->codec->blob);
                        rfree(job->codec);
                        rfree(job->stub);
                        rfree(job);
                        sem_post(&mutex);
                    };

                    job->stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    HZAPI_LOGF(INFO, "(%s) Decompressing blob of size: %lu bytes",
                               hzcodec::algorithms::algorithm_to_str(
                                       static_cast<hzcodec::algorithms::ALGORITHM>(job->codec->algorithm)),
                               src_blob->size);

                    processor->cycle();
                    processor->run(job);

                    sem_wait(&mutex);
                }

                sem_post(&mutex);
                success("Operation completed successfully");

                return;
            }
            case DECODE_CTL_MSTATE_ADDR: {
                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive was not provided");
                }

                if (mstate_addr != nullptr) {
                    rfree(mstate_addr);
                }

                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rmalloc(char, mstate_addr_len + 1);
                mstate_addr[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr, mstate_addr_len);
                break;
            }
            case DECODE_CTL_ARCHIVE: {
                if (archive_path != nullptr) {
                    rfree(archive_path);
                }

                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rmalloc(char, archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path, archive_path_len);

                archive = archive_provider->provide(archive_path);
                break;
            }
            case DECODE_CTL_SRC: {
                if (src != nullptr) {
                    rfree(src);
                }

                HZ_RECV(&src_len, sizeof(src_len));
                src = rmalloc(char, src_len + 1);
                src[src_len] = 0;

                HZ_RECV(src, src_len);

                hz_validate_path(src);
                break;
            }
            case DECODE_CTL_ALGORITHM: {
                HZ_RECV(&algorithm, sizeof(algorithm));
                break;
            }
            case DECODE_CTL_PIGGYBACK: {
                piggy_back = true;
                break;
            }
            case DECODE_CTL_MSTATE_STREAM: {
                if (mstate != nullptr) {
                    mstate->destroy();
                    rfree(mstate);
                }

                mstate = rxnew(HZ_MState);

                HZ_RECV(&mstate->length, sizeof(mstate->length));

                mstate->data = rmalloc(uint8_t, mstate->length);

                HZ_RECV(mstate->data, mstate->length);
                break;
            }
            case DECODE_CTL_BLOB_STREAM: {
                sem_wait(&mutex);
                processor->cycle();

                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggy-back was disabled");
                }

                if (mstate == nullptr && mstate_addr == nullptr) {
                    throw ApiErrors::InvalidOperationError("Mstate was not provided");
                }

                if (blob != nullptr) {
                    blob->destroy();
                    rfree(blob);
                }

                blob = rxnew(HZ_Blob);

                HZ_RECV(&blob->header.length, sizeof(blob->header.length));
                blob->header.raw = rmalloc(uint8_t, blob->header.length);
                HZ_RECV(blob->header.raw, blob->header.length);

                HZ_RECV(&blob->size, sizeof(blob->size));
                blob->data = rmalloc(uint8_t, blob->size);
                HZ_RECV(blob->data, blob->size * sizeof(uint32_t));

                HZ_RECV(&blob->o_size, sizeof(blob->o_size));

                if (mstate != nullptr) {
                    blob->mstate = mstate;
                }

                auto *job = rnew(HZ_Job);
                job->codec = rnew(HZ_CodecJob);
                job->stub = rnew(HZ_JobStub);

                if (mstate_addr != nullptr) {
                    job->codec->mstate_addr = mstate_addr;
                }
                job->codec->use_mstate_addr = mstate_addr != nullptr;
                job->codec->archive = archive;


                job->codec->blob_callback = [this](HZ_Blob *dblob) {
                    HZ_SEND(&dblob->o_size, sizeof(dblob->o_size));
                    HZ_SEND(dblob->data, dblob->o_size);
                };


                job->codec->blob = blob;
                job->codec->job_type = HZ_CodecJob::JOBTYPE::DECODE;

                job->stub->on_completed = [this, job]() {
                    rfree(job->codec);
                    rfree(job->stub);
                    rfree(job);
                    sem_post(&mutex);
                };

                job->stub->on_error = [this](const std::string &msg) {
                    error(msg);
                };

                processor->run(job);

                sem_wait(&mutex);
                sem_post(&mutex);

                success("Operation completed successfully");
                return;
            }
            default: {
                error("Invalid command");
            }
        }
    }
}

void Streamer::start() {
    uint8_t ctl_word;

    HZ_RECV(&ctl_word, sizeof(ctl_word));

    switch ((STREAM_CTL) ctl_word) {
        case STREAM_CTL_ENCODE: {
            encode();
            break;
        }
        case STREAM_CTL_DECODE: {
            decode();
            break;
        }
        case STREAM_CTL_WRITE_MSTATE: {
            write_mstate();
            break;
        }
        case STREAM_CTL_READ_MSTATE: {
            read_mstate();
            break;
        }
        default: {
            throw ApiErrors::InvalidOperationError("Invalid command");
        }
    }
}

void Streamer::write_mstate() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint8_t word{};

    char *archive_path{};
    char *mstate_addr{};

    HZ_Archive *archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((MSTATE_CTL) word) {
            case MSTATE_CTL_ARCHIVE: {
                if (archive_path != nullptr) {
                    rfree(archive_path);
                }

                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rmalloc(char, archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path, archive_path_len);

                archive = archive_provider->provide(archive_path);
                break;
            }
            case MSTATE_CTL_ADDR: {
                if (mstate_addr != nullptr) {
                    rfree(mstate_addr);
                }

                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rmalloc(char, mstate_addr_len + 1);
                mstate_addr[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr, mstate_addr_len);
                break;
            }
            case MSTATE_CTL_STREAM: {
                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive was not provided");
                }

                if (mstate_addr == nullptr) {
                    throw ApiErrors::InvalidOperationError("Mstate address was not provided");
                }

                uint64_t mstate_algorithm;
                HZ_RECV(&mstate_algorithm, sizeof(mstate_algorithm));

                uint64_t mstate_data_len;
                HZ_RECV(&mstate_data_len, sizeof(mstate_data_len));

                auto *mstate_data = rmalloc(uint8_t, mstate_data_len);
                HZ_RECV(mstate_data, mstate_data_len);

                auto *mstate = rnew(HZ_MState);
                mstate->length = mstate_data_len;
                mstate->data = mstate_data;
                mstate->alg = (hzcodec::algorithms::ALGORITHM) mstate_algorithm;

                archive->install_mstate(mstate_addr, mstate);

                success("Operation completed successfully");
                return;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}

void Streamer::read_mstate() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint8_t word{};

    char *archive_path{};
    char *mstate_addr{};
    bool piggyback = false;

    HZ_Archive *archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((MSTATE_CTL) word) {
            case MSTATE_CTL_ARCHIVE: {
                if (archive_path != nullptr) {
                    rfree(archive_path);
                }

                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rmalloc(char, archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path, archive_path_len);

                archive = archive_provider->provide(archive_path);
                break;
            }
            case MSTATE_CTL_ADDR: {
                if (mstate_addr != nullptr) {
                    rfree(mstate_addr);
                }

                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rmalloc(char, mstate_addr_len + 1);
                mstate_addr[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr, mstate_addr_len);
                break;
            }
            case MSTATE_CTL_PIGGYBACK: {
                piggyback = true;
                break;
            }
            case MSTATE_CTL_STREAM: {
                if (archive == nullptr) {
                    throw ApiErrors::InvalidOperationError("Archive was not provided");
                }

                if (mstate_addr == nullptr) {
                    throw ApiErrors::InvalidOperationError("Mstate address was not provided");
                }

                if (!piggyback) {
                    throw ApiErrors::InvalidOperationError("Null operations are not allowed");
                }

                auto *mstate = archive->read_mstate(mstate_addr);

                uint64_t alg = mstate->alg;

                HZ_SEND(&alg, sizeof(alg));
                HZ_SEND(&mstate->length, sizeof(mstate->length));
                HZ_SEND(mstate->data, mstate->length);

                mstate->destroy();
                rparentmgr->r_free(mstate);

                success("Operation completed successfully");

                return;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}
