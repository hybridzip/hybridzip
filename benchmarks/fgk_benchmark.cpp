#include <chrono>
#include <math.h>
#include <iostream>
#include "../core/entropy-coders/fgk.h"
#include "../cuda/entropy-coders/cuda_fgk.h"

void print_code(bool* code, int n) {
	for (int i = 0; i < n; i++) {
		std::cout << code[i] * 1;
	}
	std::cout << std::endl;
}

int main() {

	FGK::FGKTree tree(0x100);
	int x = 8192;

	HZIP_SIZE_T *n = new HZIP_SIZE_T;
	bool **code = new bool*;

	std::cout << "Running FGK CPU benchmark ..." << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 1024 * x; i++) {
		tree.encode(i % 256, code, n);

	}
	auto delta = (std::chrono::high_resolution_clock::now() - start).count();
	std::cout << "Time taken (CPU): " << (float)delta / 1000000000 << " s" << std::endl;

	launch_fgk_benchmark_kernel(x);

	system("pause");
	return 0;
}