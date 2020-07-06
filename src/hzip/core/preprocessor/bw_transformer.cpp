#include "bw_transformer.h"

void hztrans::bw_transformer::radix_pass(int *a, int *b, int *r, int n, int K) { // count occurrences
    int *c = new int[K + 1]; // counter array
    for (int i = 0; i <= K; i++) c[i] = 0; // reset counters
    for (int i = 0; i < n; i++) c[r[a[i]]]++; // count occurrences
    for (int i = 0, sum = 0; i <= K; i++) // exclusive prefix sums
    {
        int t = c[i];
        c[i] = sum;
        sum += t;
    }
    for (int i = 0; i < n; i++) b[c[r[a[i]]]++] = a[i]; // sort
    delete[] c;
}

void hztrans::bw_transformer::suffix_array(int *T, int *SA, int n, int K) {
    int n0 = (n + 2) / 3, n1 = (n + 1) / 3, n2 = n / 3, n02 = n0 + n2;
    int *R = new int[n02 + 3];
    R[n02] = R[n02 + 1] = R[n02 + 2] = 0;
    int *SA12 = new int[n02 + 3];
    SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;
    int *R0 = new int[n0];
    int *SA0 = new int[n0];

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

        for (int i = 0; i < n02; i++) R[SA12[i]] = i + 1;
    } else
        for (int i = 0; i < n02; i++) SA12[R[i] - 1] = i;

    for (int i = 0, j = 0; i < n02; i++) if (SA12[i] < n0) R0[j++] = 3 * SA12[i];
    radix_pass(R0, SA0, T, n0, K);

    for (int p = 0, t = n0 - n1, k = 0; k < n; k++) {
#define GetI() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)
        int i = GetI(); // pos of current offset 12 suffix
        int j = SA0[p]; // pos of current offset 0 suffix
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
    delete[] R;
    delete[] SA12;
    delete[] SA0;
    delete[] R0;
}

hztrans::bw_transformer::bw_transformer(int *data, int n, int K) {
    this->data = data;
    len = n;
    alphabet_size = K;
}

int hztrans::bw_transformer::transform() {
    int data_len = len + 1;
    int bw_index = 0;
    int *zdata = new int[data_len];
    zdata[0] = 0;

    for (int i = 0; i < len; i++) {
        zdata[i + 1] = data[i] + 1;
    }

    int *T = new int[data_len + 3];
    int *SA = new int[data_len + 3];

    for (int i = 0; i < data_len; i++) {
        T[i] = zdata[i];
        SA[i] = 1;
    }

    suffix_array(T, SA, data_len, alphabet_size + 1);

    for (int i = 0, j = 0; i < data_len; i++) {
        int val = zdata[(SA[i] + data_len - 1) % data_len];
        if (val == 0) {
            bw_index = i;
        } else {
            data[j++] = val - 1;
        }
    }

    free(zdata);

    return bw_index;
}

void hztrans::bw_transformer::invert(int bw_index) {
    int data_len = len;
    int *zdata = new int[data_len];
    int *zzdata = new int[data_len + 1];

    for (int i = 0; i < len; i++) {
        zdata[i] = data[i] + 1;
    }

    for (int i = 0, j = 0; i < data_len; i++) {
        if (i == bw_index) {
            zzdata[j++] = 0;
        }
        zzdata[j++] = zdata[i];
    }

    free(zdata);
    zdata = zzdata;

    std::vector<int> jumpers;
    auto *base_list = new std::vector<int>[alphabet_size + 1];


    for (int i = 0; i <= alphabet_size; i++) {
        base_list[i] = std::vector<int>();
    }

    for (int i = 0; i <= data_len; i++) {
        base_list[zdata[i]].push_back(i);
    }

    for (int i = 0; i <= alphabet_size; i++) {
        auto elem = base_list[i];
        if (!elem.empty()) {
            jumpers.insert(jumpers.end(), elem.begin(), elem.end());
        }
    }

    int index = bw_index;
    for (int count = 0, j = 0; count <= data_len; count++) {
        index = jumpers[index];
        if (zdata[index] != 0) {
            data[j++] = zdata[index] - 1;
        }
    }

    free(zdata);
}

