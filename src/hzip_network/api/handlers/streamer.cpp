#include "streamer.h"
#include <hzip_core/core/blob/blob.h>
#include <hzip_core/utils/validation.h>
#include <hzip_network/api/providers/archive_provider.h>
#include <hzip_network/api/api_enums.h>
#include <hzip_network/errors/api.h>

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

Streamer::Streamer(
        int sock,
        const std::string &ip_addr,
        uint16_t port,
        const rainman::ptr<HZ_Processor> &processor,
        const rainman::ptr<hzapi::ArchiveProvider> &archive_provider
) : SocketInterface(sock, ip_addr, port) {
    _processor = processor;
    _archive_provider = archive_provider;
}

void Streamer::encode() {
    uint16_t archive_path_len{};
    uint16_t mstate_addr_len{};
    uint16_t dest_len{};
    uint64_t data_len{};
    uint8_t word{};
    uint64_t algorithm{};

    rainman::ptr<char> archive_path{};
    rainman::option<rainman::ptr<char>> mstate_addr{};
    rainman::option<rainman::ptr<char>> dest{};

    bool piggy_back{};

    rainman::option<rainman::ptr<HZ_Archive>> archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((ENCODE_CTL) word) {
            case ENCODE_CTL_STREAM: {
                if (!algorithm) {
                    throw ApiErrors::InvalidOperationError("No algorithm was provided");
                }

                if (archive.is_some() && dest.is_some()) {
                    if (archive.inner()->check_file_exists(dest.inner().pointer())) {
                        throw ApiErrors::InvalidOperationError("File already exists");
                    }
                }

                if (!piggy_back && dest.is_none()) {
                    throw ApiErrors::InvalidOperationError("Null operations are not allowed");
                }

                uint64_t max_blob_size = hzes_b_size((hzcodec::algorithms::ALGORITHM) algorithm);

                HZ_RECV(&data_len, sizeof(data_len));

                std::vector<uint64_t> blob_ids;

                // Start raw-streaming using manual sync.
                HZ_RECV_SYNC;

                while (data_len > 0) {
                    _mutex->lock();

                    // Avoid processor overload.
                    _processor->cycle();

                    auto blob = rainman::ptr<HZ_Blob>();

                    max_blob_size = HZ_MIN(max_blob_size, data_len);

                    HZAPI_LOGF(INFO, "(%s) Compressing blob of size: %lu bytes", hzcodec::algorithms::algorithm_to_str(
                            static_cast<hzcodec::algorithms::ALGORITHM>(algorithm)), max_blob_size);

                    blob->o_size = max_blob_size;
                    blob->data = rainman::ptr<uint8_t>(max_blob_size);

                    // Enable raw streaming in hzip_core-protocol
                    t_recv(blob->data.pointer(), max_blob_size, false);

                    // Construct hz_job struct for processing.
                    auto job = rainman::ptr<HZ_Job>();
                    auto codec = rainman::ptr<HZ_CodecJob>();
                    codec->algorithm = (hzcodec::algorithms::ALGORITHM) algorithm;
                    codec->archive = archive;
                    if (mstate_addr.is_some()) {
                        codec->mstate_addr = std::string(mstate_addr.inner().pointer());
                    }
                    codec->blob = blob;

                    if (archive.is_some()) {
                        codec->blob_id_callback = [&blob_ids](uint64_t id) {
                            blob_ids.push_back(id);
                        };
                    }

                    codec->job_type = HZ_CodecJob::CodecJobType::ENCODE;

                    job->codec = codec;

                    if (piggy_back) {
                        codec->blob_callback = [this](const rainman::ptr<HZ_Blob> &cblob) {
                            uint64_t alg = cblob->mstate->alg;
                            uint64_t mstate_len = cblob->mstate->size();
                            uint64_t header_len = cblob->header.size();
                            HZ_SEND(&alg, sizeof(alg));
                            HZ_SEND(&mstate_len, sizeof(mstate_len));
                            HZ_SEND(cblob->mstate->data.pointer(), mstate_len);

                            HZ_SEND(&header_len, sizeof(header_len));
                            HZ_SEND(cblob->header.raw.pointer(), header_len);

                            HZ_SEND(&cblob->size, sizeof(cblob->size));
                            HZ_SEND(cblob->data.pointer(), cblob->size);
                        };
                    }


                    auto stub = rainman::ptr<HZ_JobStub>();
                    stub->on_completed = [this]() {
                        _mutex->unlock();
                    };

                    stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    job->stub = stub;
                    job->type = HZ_Job::CODEC;

                    // Dispatch hz_job to hz_processor.
                    _processor->run(job);
                    data_len -= max_blob_size;
                }


                _mutex->lock();

                if (archive.is_some() && dest.is_some()) {
                    // Allocate outside the scope of the rainman module for persistence.
                    auto blob_id_arr = rainman::ptr<uint64_t>(blob_ids.size());
                    for (int i = 0; i < blob_ids.size(); i++) {
                        blob_id_arr[i] = blob_ids[i];
                    }

                    HZ_ArchiveFile file{};
                    file.blob_ids = blob_id_arr;
                    file.blob_count = blob_ids.size();

                    archive.inner()->create_file_entry(dest.inner().pointer(), file);
                }

                _mutex->unlock();

                success("Operation completed successfully");

                return;
            }
            case ENCODE_CTL_MSTATE_ADDR: {
                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rainman::ptr<char>(mstate_addr_len + 1);
                mstate_addr.inner()[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr.inner().pointer(), mstate_addr_len);
                break;
            }
            case ENCODE_CTL_ARCHIVE: {
                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rainman::ptr<char>(archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path.pointer(), archive_path_len);

                archive = _archive_provider->provide(archive_path.pointer());
                break;
            }
            case ENCODE_CTL_DEST: {
                HZ_RECV(&dest_len, sizeof(dest_len));

                dest = rainman::ptr<char>(dest_len + 1);
                dest.inner()[dest_len] = 0;

                HZ_RECV(dest.inner().pointer(), dest_len);

                hz_validate_path(dest.inner().pointer());
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

                if (mstate_addr.is_none()) {
                    throw ApiErrors::InvalidOperationError("Mstate address not found");
                }

                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not found");
                }

                HZ_RECV(&data_len, sizeof(data_len));

                uint64_t max_blob_size = hzes_b_size((hzcodec::algorithms::ALGORITHM) algorithm);

                // Start raw-streaming using manual sync.
                HZ_RECV_SYNC;

                uint64_t batch_count = 0;

                while (data_len > 0) {
                    _mutex->lock();
                    HZAPI_LOGF(INFO, "(%s) Training: '%s' - Batch: %lu", hzcodec::algorithms::algorithm_to_str(
                            static_cast<hzcodec::algorithms::ALGORITHM>(algorithm)), mstate_addr.inner().pointer(),
                               ++batch_count);

                    // Avoid processor overload.
                    _processor->cycle();

                    auto blob = rainman::ptr<HZ_Blob>();

                    max_blob_size = HZ_MIN(max_blob_size, data_len);

                    blob->o_size = max_blob_size;
                    blob->data = rainman::ptr<uint8_t>(max_blob_size);

                    // Enable raw streaming in hzip_core-protocol
                    t_recv(blob->data.pointer(), max_blob_size, false);

                    // Construct hz_job struct for processing.
                    auto job = rainman::ptr<HZ_Job>();
                    auto codec = rainman::ptr<HZ_CodecJob>();
                    auto stub = rainman::ptr<HZ_JobStub>();

                    codec->algorithm = (hzcodec::algorithms::ALGORITHM) algorithm;
                    codec->archive = archive;
                    if (mstate_addr.is_some()) {
                        codec->mstate_addr = std::string(mstate_addr.inner().pointer());
                    }
                    codec->blob = blob;
                    codec->job_type = HZ_CodecJob::TRAIN;


                    stub->on_completed = [this]() {
                        _mutex->unlock();
                    };

                    stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    job->codec = codec;
                    job->stub = stub;

                    // Dispatch hz_job to hz_processor.
                    _processor->run(job);

                    data_len -= max_blob_size;
                }

                _mutex->lock();
                _mutex->unlock();

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

    rainman::option<rainman::ptr<HZ_MState>> mstate{};
    rainman::ptr<HZ_Blob> blob{};

    rainman::ptr<char> archive_path{};
    rainman::option<rainman::ptr<char>> mstate_addr{};
    rainman::option<rainman::ptr<char>> src{};

    bool piggy_back{};

    rainman::option<rainman::ptr<HZ_Archive>> archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((DECODE_CTL) word) {
            case DECODE_CTL_STREAM: {
                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggy-back was disabled");
                }

                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive not provided");
                }

                if (src.is_none()) {
                    throw ApiErrors::InvalidOperationError("Source not provided");
                }

                _mutex->lock();

                auto file_entry = archive.inner()->read_file_entry(src.inner().pointer());

                HZ_SEND(&file_entry.blob_count, sizeof(file_entry.blob_count));

                for (uint64_t i = 0; i < file_entry.blob_count; i++) {
                    auto src_blob = archive.inner()->read_blob(file_entry.blob_ids[i]);

                    // Construct hz_job

                    auto job = rainman::ptr<HZ_Job>();
                    auto codec = rainman::ptr<HZ_CodecJob>();
                    auto stub = rainman::ptr<HZ_JobStub>();

                    codec->archive = archive;
                    codec->job_type = HZ_CodecJob::DECODE;
                    codec->blob = src_blob;
                    codec->algorithm = src_blob->mstate->alg;

                    codec->blob_callback = [this](const rainman::ptr<HZ_Blob> &dblob) {
                        HZ_SEND(&dblob->o_size, sizeof(dblob->o_size));
                        HZ_SEND(dblob->data.pointer(), dblob->o_size);
                    };

                    stub->on_completed = [this]() {
                        _mutex->unlock();
                    };

                    stub->on_error = [this](const std::string &msg) {
                        error(msg);
                    };

                    HZAPI_LOGF(INFO, "(%s) Decompressing blob of size: %lu bytes",
                               hzcodec::algorithms::algorithm_to_str(
                                       static_cast<hzcodec::algorithms::ALGORITHM>(codec->algorithm)),
                               src_blob->size);

                    job->codec = codec;
                    job->stub = stub;

                    _processor->cycle();
                    _processor->run(job);

                    _mutex->lock();
                }

                _mutex->unlock();
                success("Operation completed successfully");

                return;
            }
            case DECODE_CTL_MSTATE_ADDR: {
                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive was not provided");
                }
                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rainman::ptr<char>(mstate_addr_len + 1);
                mstate_addr.inner()[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr.inner().pointer(), mstate_addr_len);
                break;
            }
            case DECODE_CTL_ARCHIVE: {
                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rainman::ptr<char>(archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path.pointer(), archive_path_len);

                archive = _archive_provider->provide(archive_path.pointer());
                break;
            }
            case DECODE_CTL_SRC: {
                HZ_RECV(&src_len, sizeof(src_len));
                src = rainman::ptr<char>(src_len + 1);
                src.inner()[src_len] = 0;

                HZ_RECV(src.inner().pointer(), src_len);

                hz_validate_path(src.inner().pointer());
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
                mstate = rainman::ptr<HZ_MState>();
                uint64_t mstate_len = 0;

                HZ_RECV(&mstate_len, sizeof(mstate_len));

                mstate.inner()->data = rainman::ptr<uint8_t>(mstate_len);

                HZ_RECV(mstate.inner()->data.pointer(), mstate_len);
                break;
            }
            case DECODE_CTL_BLOB_STREAM: {
                _mutex->lock();
                _processor->cycle();

                if (!piggy_back) {
                    throw ApiErrors::InvalidOperationError("Piggy-back was disabled");
                }

                if (mstate.is_none() && mstate_addr.is_none()) {
                    throw ApiErrors::InvalidOperationError("Mstate was not provided");
                }

                blob = rainman::ptr<HZ_Blob>();
                uint64_t header_len = 0;

                HZ_RECV(&header_len, sizeof(header_len));
                blob->header.raw = rainman::ptr<uint8_t>(header_len);
                HZ_RECV(blob->header.raw.pointer(), header_len);

                HZ_RECV(&blob->size, sizeof(blob->size));
                blob->data = rainman::ptr<uint8_t>(blob->size);
                HZ_RECV(blob->data.pointer(), blob->size);

                HZ_RECV(&blob->o_size, sizeof(blob->o_size));

                if (mstate.is_some()) {
                    blob->mstate = mstate.inner();
                }

                auto job = rainman::ptr<HZ_Job>();
                auto codec = rainman::ptr<HZ_CodecJob>();
                auto stub = rainman::ptr<HZ_JobStub>();

                if (mstate_addr.is_some()) {
                    codec->mstate_addr = std::string(mstate_addr.inner().pointer());
                }
                codec->archive = archive;


                codec->blob_callback = [this](const rainman::ptr<HZ_Blob> &dblob) {
                    HZ_SEND(&dblob->o_size, sizeof(dblob->o_size));
                    HZ_SEND(dblob->data.pointer(), dblob->o_size);
                };


                codec->blob = blob;
                codec->job_type = HZ_CodecJob::DECODE;

                job->stub->on_completed = [this]() {
                    _mutex->unlock();
                };

                job->stub->on_error = [this](const std::string &msg) {
                    error(msg);
                };

                _processor->run(job);

                _mutex->lock();
                _mutex->unlock();

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

    rainman::ptr<char> archive_path{};
    rainman::option<rainman::ptr<char>> mstate_addr{};

    rainman::option<rainman::ptr<HZ_Archive>> archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((MSTATE_CTL) word) {
            case MSTATE_CTL_ARCHIVE: {
                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rainman::ptr<char>(archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path.pointer(), archive_path_len);

                archive = _archive_provider->provide(archive_path.pointer());
                break;
            }
            case MSTATE_CTL_ADDR: {
                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rainman::ptr<char>(mstate_addr_len + 1);
                mstate_addr.inner()[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr.inner().pointer(), mstate_addr_len);
                break;
            }
            case MSTATE_CTL_STREAM: {
                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive was not provided");
                }

                if (mstate_addr.is_none()) {
                    throw ApiErrors::InvalidOperationError("Mstate address was not provided");
                }

                uint64_t mstate_algorithm;
                HZ_RECV(&mstate_algorithm, sizeof(mstate_algorithm));

                uint64_t mstate_data_len;
                HZ_RECV(&mstate_data_len, sizeof(mstate_data_len));

                auto mstate_data = rainman::ptr<uint8_t>(mstate_data_len);
                HZ_RECV(mstate_data.pointer(), mstate_data_len);

                auto mstate = rainman::ptr<HZ_MState>();
                mstate->data = mstate_data;
                mstate->alg = (hzcodec::algorithms::ALGORITHM) mstate_algorithm;

                archive.inner()->install_mstate(mstate_addr.inner().pointer(), mstate);

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

    rainman::ptr<char> archive_path{};
    rainman::option<rainman::ptr<char>> mstate_addr{};
    bool piggyback = false;

    rainman::option<rainman::ptr<HZ_Archive>> archive{};

    while (true) {
        HZ_RECV(&word, sizeof(word));

        switch ((MSTATE_CTL) word) {
            case MSTATE_CTL_ARCHIVE: {
                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rainman::ptr<char>(archive_path_len + 1);
                archive_path[archive_path_len] = 0;

                HZ_RECV(archive_path.pointer(), archive_path_len);

                archive = _archive_provider->provide(archive_path.pointer());
                break;
            }
            case MSTATE_CTL_ADDR: {
                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rainman::ptr<char>(mstate_addr_len + 1);
                mstate_addr.inner()[mstate_addr_len] = 0;

                HZ_RECV(mstate_addr.inner().pointer(), mstate_addr_len);
                break;
            }
            case MSTATE_CTL_PIGGYBACK: {
                piggyback = true;
                break;
            }
            case MSTATE_CTL_STREAM: {
                if (archive.is_none()) {
                    throw ApiErrors::InvalidOperationError("Archive was not provided");
                }

                if (mstate_addr.is_none()) {
                    throw ApiErrors::InvalidOperationError("Mstate address was not provided");
                }

                if (!piggyback) {
                    throw ApiErrors::InvalidOperationError("Null operations are not allowed");
                }

                auto mstate = archive.inner()->read_mstate(mstate_addr.inner().pointer());

                uint64_t alg = mstate->alg;
                uint64_t mstate_len = 0;

                HZ_SEND(&alg, sizeof(alg));
                HZ_SEND(&mstate_len, sizeof(mstate_len));
                HZ_SEND(mstate->data, mstate_len);

                success("Operation completed successfully");

                return;
            }
            default: {
                throw ApiErrors::InvalidOperationError("Invalid command");
            }
        }
    }
}
