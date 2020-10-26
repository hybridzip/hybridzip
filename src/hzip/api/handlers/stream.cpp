#include "stream.h"
#include <hzip/api/providers/archive_provider.h>
#include <hzip/utils/validation.h>
#include <hzip/api/api_enums.h>
#include <hzip/errors/api.h>

using namespace hzapi;

#define HZ_MIN(a, b) (a) < (b) ? (a) : (b)

uint64_t hz_streamer::hzes_b_size(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNDEFINED:
            return 0xffffffffffffffff;
        case hzcodec::algorithms::VICTINI:
            return 0x400000;
        default:
            return 0xffffffffffffffff;
    }
}

hz_streamer::hz_streamer(int _sock, char *_ip_addr, uint16_t _port, hz_processor *_proc,
                         hzprovider::archive *_archive_provider) {
    sock = _sock;
    ip_addr = _ip_addr;
    port = _port;
    processor = _proc;
    archive_provider = _archive_provider;
    sem_init(&mutex, 0, 1);
}

void hz_streamer::encode() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint16_t dest_len{};
    uint64_t data_len{};
    uint8_t word{};
    uint8_t algorithm{};

    char *archive_path{};
    char *mstate_addr{};
    char *dest{};

    bool piggy_back{};

    hz_archive *archive{};

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

                    auto *blob = rxnew(hzblob_t);

                    max_blob_size = HZ_MIN(max_blob_size, data_len);

                    blob->o_size = max_blob_size;
                    blob->o_data = rmalloc(uint8_t, max_blob_size);

                    // Enable raw streaming in hzip-protocol
                    t_recv(blob->o_data, max_blob_size, false);

                    // Construct hz_job struct for processing.
                    auto *job = rnew(hz_job);
                    job->codec = rnew(hz_codec_job);

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
                        job->codec->blob_callback = [this](hzblob_t *cblob) {
                            HZ_SEND(&cblob->mstate->alg, sizeof(cblob->mstate->alg));
                            HZ_SEND(&cblob->mstate->length, sizeof(cblob->mstate->length));
                            HZ_SEND(cblob->mstate->data, cblob->mstate->length);

                            HZ_SEND(&cblob->header.length, sizeof(cblob->header.length));
                            HZ_SEND(cblob->header.raw, cblob->header.length);

                            HZ_SEND(&cblob->size, sizeof(cblob->size));
                            HZ_SEND(cblob->data, sizeof(uint32_t) * cblob->size);
                        };
                    }

                    job->codec->job_type = hz_codec_job::JOBTYPE::ENCODE;

                    job->stub = rnew(hz_job_stub);
                    job->stub->on_completed = [this, job]() {
                        rfree(job->codec);
                        rfree(job->stub);
                        rfree(job);
                        sem_post(&mutex);
                    };

                    job->stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    job->stub->on_success = [this](const std::string &msg) {
                        success(msg);
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

                    hza_file file{};
                    file.blob_ids = blob_id_arr;
                    file.blob_count = blob_ids.size();

                    archive->create_file_entry(dest, file);
                }

                sem_post(&mutex);

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

                if (archive->check_mstate_exists(mstate_addr)) {
                    throw ApiErrors::InvalidOperationError("Incremental mstate training is not supported");
                }

                //todo: Support incremental mstate-training

                HZ_RECV(&data_len, sizeof(data_len));

                // Start raw-streaming using manual sync.
                HZ_RECV_SYNC;

                sem_wait(&mutex);

                // Avoid processor overload.
                processor->cycle();

                auto *blob = rxnew(hzblob_t);

                blob->o_size = data_len;
                blob->o_data = rmalloc(uint8_t, data_len);

                // Enable raw streaming in hzip-protocol
                t_recv(blob->o_data, data_len, false);

                // Construct hz_job struct for processing.
                auto *job = rnew(hz_job);
                job->codec = rnew(hz_codec_job);

                job->codec->algorithm = (hzcodec::algorithms::ALGORITHM) algorithm;
                job->codec->archive = archive;
                job->codec->use_mstate_addr = mstate_addr != nullptr;
                if (mstate_addr != nullptr) {
                    job->codec->mstate_addr = mstate_addr;
                }
                job->codec->blob = blob;

                job->codec->job_type = hz_codec_job::JOBTYPE::TRAIN;

                job->stub = rnew(hz_job_stub);
                job->stub->on_completed = [this, job]() {
                    rfree(job->codec);
                    rfree(job->stub);
                    rfree(job);
                    sem_post(&mutex);
                };

                job->stub->on_error = [this](const std::string &msg) {
                    error(msg);
                };

                job->stub->on_success = [this](const std::string &msg) {
                    success(msg);
                };

                // Dispatch hz_job to hz_processor.
                processor->run(job);
            }
            default: {
                error("Invalid command");
            }
        }
    }
}

void hz_streamer::decode() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint16_t src_len{};
    uint8_t word{};
    uint8_t algorithm{};

    hz_mstate *mstate{};
    hzblob_t *blob{};

    char *archive_path{};
    char *mstate_addr{};
    char *src{};

    bool piggy_back{};

    hz_archive *archive{};

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

                    auto *job = rnew(hz_job);
                    job->codec = rnew(hz_codec_job);
                    job->stub = rnew(hz_job_stub);

                    job->codec->archive = archive;
                    job->codec->job_type = hz_codec_job::JOBTYPE::DECODE;
                    job->codec->blob = src_blob;
                    job->codec->use_mstate_addr = false;
                    job->codec->algorithm = src_blob->mstate->alg;

                    job->codec->blob_callback = [this](hzblob_t *dblob) {
                        HZ_SEND(&dblob->o_size, sizeof(dblob->o_size));
                        HZ_SEND(dblob->o_data, dblob->o_size);
                    };

                    job->stub->on_completed = [this, job]() {
                        rfree(job->codec);
                        rfree(job->stub);
                        rfree(job);
                        sem_post(&mutex);
                    };

                    job->stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    job->stub->on_success = [this](const std::string &msg) {
                        success(msg);
                    };

                    processor->cycle();
                    processor->run(job);

                    sem_wait(&mutex);
                }

                sem_post(&mutex);

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

                mstate = rxnew(hz_mstate);

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

                blob = rxnew(hzblob_t);

                HZ_RECV(&blob->header.length, sizeof(blob->header.length));
                blob->header.raw = rmalloc(uint8_t, blob->header.length);
                HZ_RECV(blob->header.raw, blob->header.length);

                HZ_RECV(&blob->size, sizeof(blob->size));
                blob->data = rmalloc(uint32_t, blob->size);
                HZ_RECV(blob->data, blob->size * sizeof(uint32_t));

                HZ_RECV(&blob->o_size, sizeof(blob->o_size));

                if (mstate != nullptr) {
                    blob->mstate = mstate;
                }

                auto *job = rnew(hz_job);
                job->codec = rnew(hz_codec_job);
                job->stub = rnew(hz_job_stub);

                if (mstate_addr != nullptr) {
                    job->codec->mstate_addr = mstate_addr;
                }
                job->codec->use_mstate_addr = mstate_addr != nullptr;
                job->codec->archive = archive;


                job->codec->blob_callback = [this](hzblob_t *dblob) {
                    HZ_SEND(&dblob->o_size, sizeof(dblob->o_size));
                    HZ_SEND(dblob->o_data, dblob->o_size);
                };


                job->codec->blob = blob;
                job->codec->job_type = hz_codec_job::JOBTYPE::DECODE;

                job->stub->on_completed = [this, job]() {
                    rfree(job->codec);
                    rfree(job->stub);
                    rfree(job);
                    sem_post(&mutex);
                };

                job->stub->on_error = [this](const std::string &msg) {
                    error(msg);
                };

                job->stub->on_success = [this](const std::string &msg) {
                    success(msg);
                };

                processor->run(job);

                return;
            }
            default: {
                error("Invalid command");
            }
        }
    }
}

void hz_streamer::start() {
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

void hz_streamer::write_mstate() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint8_t word{};

    char *archive_path{};
    char *mstate_addr{};

    hz_archive *archive{};

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

                uint8_t mstate_algorithm;
                HZ_RECV(&mstate_algorithm, sizeof(mstate_algorithm));

                uint64_t mstate_data_len;
                HZ_RECV(&mstate_data_len, sizeof(mstate_data_len));

                auto *mstate_data = rmalloc(uint8_t, mstate_data_len);
                HZ_RECV(mstate_data, mstate_data_len);

                auto *mstate = rnew(hz_mstate);
                mstate->length = mstate_data_len;
                mstate->data = mstate_data;
                mstate->alg = (hzcodec::algorithms::ALGORITHM) mstate_algorithm;

                archive->install_mstate(mstate_addr, mstate);

                return;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}

void hz_streamer::read_mstate() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint8_t word{};

    char *archive_path{};
    char *mstate_addr{};
    bool piggyback = false;

    hz_archive *archive{};

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

                HZ_SEND(&mstate->alg, sizeof(mstate->alg));
                HZ_SEND(&mstate->length, sizeof(mstate->length));
                HZ_SEND(mstate->data, mstate->length);

                return;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}
