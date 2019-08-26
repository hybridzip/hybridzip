#include <chrono>
#include <math.h>
#include "host/bitio.h"
#include "cuda/gpu_info.h"

int main() {
	//detect_gpus();
	BitIO bitio(_strdup("F:/sample.txt"), 1);
	bitio.skip(16);
	std::cout << bitio.read(8) << std::endl;
	bitio.write(98, 8);
	bitio.flush();
	bitio.close();
    system("pause");
    return 0;
}