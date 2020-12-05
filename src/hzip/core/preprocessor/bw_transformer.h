#ifndef HYBRIDZIP_BW_TRANSFORMER_H
#define HYBRIDZIP_BW_TRANSFORMER_H

#include <vector>
#include <cstdint>
#include <rainman/rainman.h>

namespace hztrans {
    template<typename itype, typename mtype>
    class bw_transformer : public rainman::context {

    private:
        static inline bool leq(mtype a1, mtype a2, mtype b1, mtype b2) {
            return (a1 < b1 || a1 == b1 && a2 <= b2);
        }

        static inline bool leq(mtype a1, mtype a2, mtype a3, mtype b1, mtype b2, mtype b3) {
            return (a1 < b1 || a1 == b1 && leq(a2, a3, b2, b3));
        }

        void radix_pass(mtype *a, mtype *b, mtype *r, uint64_t n, uint64_t K) { // count occurrences
            auto c = rmalloc(mtype, K + 1); // counter array
            for (int i = 0; i <= K; i++) c[i] = 0; // reset counters
            for (int i = 0; i < n; i++) {
                c[r[a[i]]]++; // count occurrences
            }
            for (int i = 0, sum = 0; i <= K; i++) {
                int t = c[i];
                c[i] = sum;
                sum += t;
            }
            for (int i = 0; i < n; i++) b[c[r[a[i]]]++] = a[i]; // sort

            rfree(c);
        }

        void suffix_array(mtype *T, mtype *SA, uint64_t n, uint64_t K) {
            int n0 = (n + 2) / 3, n1 = (n + 1) / 3, n2 = n / 3, n02 = n0 + n2;
            auto R = rmalloc(mtype, n02 + 3);
            R[n02] = R[n02 + 1] = R[n02 + 2] = 0;
            auto SA12 = rmalloc(mtype, n02 + 3);
            SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;
            auto R0 = rmalloc(mtype, n0);
            auto SA0 = rmalloc(mtype, n0);

            for (int i = 0, j = 0; i < n + (n0 - n1); i++) if (i % 3 != 0) R[j++] = i;

            radix_pass(R, SA12, T + 2, n02, K);
            radix_pass(SA12, R, T + 1, n02, K);
            radix_pass(R, SA12, T, n02, K);

            int name = 0, c0 = -1, c1 = -1, c2 = -1;
            for (int i = 0; i < n02; i++) {
                if (T[SA12[i]] != c0 || T[SA12[i] + 1] != c1 || T[SA12[i] + 2] != c2) {
                    name++;
                    c0 = T[SA12[i]];
                    c1 = T[SA12[i] + 1];
                    c2 = T[SA12[i] + 2];
                }
                if (SA12[i] % 3 == 1) { R[SA12[i] / 3] = name; } // write to R1
                else { R[SA12[i] / 3 + n0] = name; } // write to R2
            }

            if (name < n02) {
                suffix_array(R, SA12, n02, name);

                for (uint64_t i = 0; i < n02; i++) R[SA12[i]] = i + 1;
            } else
                for (uint64_t i = 0; i < n02; i++) SA12[R[i] - 1] = i;

            for (uint64_t i = 0, j = 0; i < n02; i++) if (SA12[i] < n0) R0[j++] = 3 * SA12[i];
            radix_pass(R0, SA0, T, n0, K);

            for (uint64_t p = 0, t = n0 - n1, k = 0; k < n; k++) {
#define GetI() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)
                mtype i = GetI(); // pos of current offset 12 suffix
                mtype j = SA0[p]; // pos of current offset 0 suffix
                if (SA12[t] < n0 ? // different compares for mod 1 and mod 2 suffixes
                    leq(T[i], R[SA12[t] + n0], T[j], R[j / 3]) :
                    leq(T[i], T[i + 1], R[SA12[t] - n0 + 1], T[j], T[j + 1],
                        R[j / 3 + n0])) { // suffix from SA12 is smaller
                    SA[k] = i;
                    t++;
                    if (t == n02) // done --- only SA0 suffixes left
                        for (k++; p < n0; p++, k++) SA[k] = SA0[p];
                } else { // suffix from SA0 is smaller
                    SA[k] = j;
                    p++;
                    if (p == n0) // done --- only SA12 suffixes left
                        for (k++; t < n02; t++, k++) SA[k] = GetI();
                }
            }
            rfree(R);
            rfree(SA12);
            rfree(SA0);
            rfree(R0);
        }

        itype *data;
        uint64_t len, alphabet_size;
    public:
        bw_transformer(itype *data, uint64_t n, uint64_t K) {
            this->data = data;
            len = n;
            alphabet_size = K;
        }

        uint64_t transform() {
            uint64_t data_len = len + 1;
            uint64_t bw_index = 0;
            auto zdata = rmalloc(itype, data_len);
            zdata[0] = 0;

            for (int i = 0; i < len; i++) {
                zdata[i + 1] = data[i] + 1;
            }

            auto T = rmalloc(mtype, data_len + 3);
            auto SA = rmalloc(mtype, data_len + 3);

            for (uint64_t i = 0; i < data_len + 3; i++) {
                T[i] = 0;
                SA[i] = 0;
            }

            for (uint64_t i = 0; i < data_len; i++) {
                T[i] = zdata[i];
                SA[i] = 1;
            }

            suffix_array(T, SA, data_len, alphabet_size + 1);

            rfree(T);

            for (uint64_t i = 0, j = 0; i < data_len; i++) {
                int val = zdata[(SA[i] + data_len - 1) % data_len];
                if (val == 0) {
                    bw_index = i;
                } else {
                    data[j++] = val - 1;
                }
            }

            rfree(zdata);
            rfree(SA);

            return bw_index;
        }

        void invert(uint64_t bw_index) {
            int data_len = len;
            auto zdata = rmalloc(itype, data_len);
            auto zzdata = rmalloc(itype, data_len + 1);

            for (int i = 0; i < len; i++) {
                zdata[i] = data[i] + 1;
            }

            for (uint64_t i = 0, j = 0; i <= data_len; i++) {
                if (i == bw_index) {
                    zzdata[j++] = 0;
                }

                if (i < data_len) {
                    zzdata[j++] = zdata[i];
                }
            }

            rfree(zdata);
            zdata = zzdata;

            std::vector<int> jumpers;
            auto base_list = rmalloc(std::vector<uint64_t>, alphabet_size + 1);

            for (int i = 0; i <= alphabet_size; i++) {
                base_list[i] = std::vector<uint64_t>();
            }

            for (uint64_t i = 0; i <= data_len; i++) {
                base_list[zdata[i]].push_back(i);
            }

            for (int i = 0; i <= alphabet_size; i++) {
                auto elem = base_list[i];
                if (!elem.empty()) {
                    jumpers.insert(jumpers.end(), elem.begin(), elem.end());
                }
            }

            rfree(base_list);

            uint64_t index = bw_index;
            for (uint64_t count = 0, j = 0; count <= data_len; count++) {
                index = jumpers[index];
                if (zdata[index] != 0) {
                    data[j++] = zdata[index] - 1;
                }
            }

            rfree(zdata);
        }
    };
}

#endif