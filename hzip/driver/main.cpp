#include <chrono>
#include <math.h>
#include <iostream>
#include <random>
#include "../core/middle-out/context_graph.h"
#include "../bitio/bitio_buffer.h"

int main() {

	middleout::SingleOrderContext soc;
	bitio::BitIOBuffer buf(1024);
	soc.setBitIOBuffer(&buf);

	auto start = std::chrono::high_resolution_clock::now();
	int multiplier = 1024;
	for (int elem = 0; elem < 1024 * multiplier; elem++) {
		soc.feed(rand() * 255 / RAND_MAX);
	}

	std::cout << "SingleOrderContext - benchmark: " << (float)1000000000 * multiplier / (float)(std::chrono::high_resolution_clock::now() - start).count() << " Kbps" << std::endl;
	soc.~SingleOrderContext();

	buf.flush(fopen("F:/sample.soc", "w"));
	
    system("pause");
    return 0;
}