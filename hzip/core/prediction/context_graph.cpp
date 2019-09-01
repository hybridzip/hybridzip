#include "context_graph.h"

using namespace predictors;

SingleContextGraph::SingleContextGraph() {
	univ.tree = FGK::FGKTree(0x100);
	pivots = (cnode_t*)malloc(sizeof(cnode_t) * 0x100);

	for (int i = 0; i < 0x100; i++) {
		pivots[i].tree = FGK::FGKTree(0x101);
		univ.freq[i] = 0;
	}

	prev = 0x100;
}

SingleContextGraph::~SingleContextGraph() {
	free(pivots);
}

void SingleContextGraph::setBitIO(BitIO *bitio) {
	this->bitio = bitio;
}

void SingleContextGraph::feed(HZIP_UINT elem) {
	if (prev == 0x100) {
		bitio->write(elem, 8);
	}
	else if(univ.freq[elem] == 0){
		univ.tree.encode(elem);
		univ.freq[elem]++;

		pivots[prev].tree.encode(elem);
		pivots[prev].freq[elem]++;

		bitio->write(elem, 8);
	}
	else if (pivots[prev].freq[elem] == 0){
		pivots[prev].tree.encode(elem);
		pivots[prev].freq[elem]++;

		auto bin = univ.tree.encode(elem);
		univ.freq[elem]++;
		bitio->write(bin.obj, bin.n);
	}
	else {
		univ.freq[elem]++;
		univ.tree.encode(elem);

		auto bin = pivots[prev].tree.encode(elem);
		pivots[prev].freq[elem]++;

		bitio->write(bin.obj, bin.n);
	}

	prev = elem;
}

