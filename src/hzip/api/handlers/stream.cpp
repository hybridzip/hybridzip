#include "stream.h"
#include <hzip/api/providers/archive_provider.h>
#include <hzip/utils/validation.h>
#include <hzip/errors/api.h>

#define HZ_MIN(a, b) (a) < (b) ? (a) : (b)

uint64_t hz_streamer::hzes_b_size(hzcodec::algorithms::ALGORITHM alg) {
    switch (alg) {
        case hzcodec::algorithms::UNDEFINED:
            return 0xffffffffffffffff;
        case hzcodec::algorithms::VICTINI:
            return 0x400000;
    }
}

hz_streamer::hz_streamer(int _sock, char *_ip_addr, uint16_t _port, hz_processor *_proc) {
    sock = _sock;
    ip_addr = _ip_addr;
    port = _port;
    processor = _proc;
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

                uint64_t max_blob_size = hzes_b_size((hzcodec::algorithms::ALGORITHM) algorithm);

                HZ_RECV(&data_len, sizeof(data_len));

                std::vector<uint64_t> blob_ids;

                while (data_len > 0) {
                    sem_wait(&mutex);

                    auto *blob = rxnew(hzblob_t);

                    max_blob_size = HZ_MIN(max_blob_size, data_len);

                    blob->o_size = max_blob_size;
                    blob->o_data = rmalloc(uint8_t, max_blob_size);

                    HZ_RECV(blob->o_data, max_blob_size);

                    // Construct hz_job struct for processing.
                    auto *job = rnew(hz_job);
                    job->codec = rnew(hz_codec_job);

                    job->codec->algorithm = (hzcodec::algorithms::ALGORITHM) algorithm;
                    job->codec->archive = archive;
                    job->codec->mstate_addr = mstate_addr;
                    job->codec->reuse_mstate = mstate_addr != nullptr;
                    job->codec->blob = blob;

                    if (archive != nullptr) {
                        job->codec->blob_id_callback = [&blob_ids](uint64_t id) {
                            blob_ids.push_back(id);
                        };
                    }

                    if (piggy_back) {
                        job->codec->blob_callback = [this](hzblob_t *cblob) {
                            HZ_SEND(&cblob->mstate->alg, sizeof(cblob->mstate->alg));
                            HZ_SEND(&cblob->size, sizeof(cblob->size));
                            HZ_SEND(cblob->data, sizeof(uint32_t) * cblob->size);
                            HZ_SEND(&cblob->mstate->length, sizeof(cblob->mstate->length));
                            HZ_SEND(cblob->mstate->data, cblob->mstate->length);
                        };
                    }

                    job->codec->job_type = hz_codec_job::JOBTYPE::ENCODE;

                    job->stub = rnew(hz_job_stub);
                    job->stub->on_completed = [this]() {
                        sem_post(&mutex);
                    };

                    // Dispatch hz_job to hz_processor.
                    processor->run(job);
                    data_len -= max_blob_size;
                }


                sem_wait(&mutex);

                if (archive != nullptr && dest != nullptr) {
                    auto *blob_id_arr = rmalloc(uint64_t, blob_ids.size());
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
                HZ_RECV(&mstate_addr_len, sizeof(mstate_addr_len));

                mstate_addr = rmalloc(char, mstate_addr_len);
                HZ_RECV(mstate_addr, mstate_addr_len);
                break;
            }
            case ENCODE_CTL_ARCHIVE: {
                HZ_RECV(&archive_path_len, sizeof(archive_path_len));
                archive_path = rmalloc(char, archive_path_len);
                HZ_RECV(archive_path, archive_path_len);

                archive = hzprovider::archive::provide(archive_path);
                break;
            }
            case ENCODE_CTL_DEST: {
                HZ_RECV(&dest_len, sizeof(dest_len));

                dest = rmalloc(char, dest_len);
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
        }
    }
}
