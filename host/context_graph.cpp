#include "context_graph.h"

ContextCradle::ContextCradle(size_t alphabet_size, float learning_rate, size_t ncontexts) {
	this->learning_rate = learning_rate;
	this->alphabet_size = alphabet_size;
	k = ncontexts;
	context_vectors = (context_vector*)malloc(sizeof(context_vector) * ncontexts);
	for (int i = 0; i < ncontexts; i++) {
		context_vectors[i].context_map = (size_t*)malloc(sizeof(size_t) * alphabet_size * alphabet_size);
		context_vectors[i].weight = 1;
	}
	// initialize circular queue.
	
}

size_t ContextCradle::feed(size_t obj) {
	if (front == -1 && rear == -1)
		front = rear = 0;
	else {
		obj_buf[rear++] = obj;
		rear %= k;
	}
}


