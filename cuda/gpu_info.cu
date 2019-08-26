#include "gpu_info.h"

void detect_gpus() {
	int count;
	cudaGetDeviceCount(&count);
	printf("Detected GPU(s): %d\n", count);
	
}