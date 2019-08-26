#include <malloc.h>
#include "ringbuffer.h"

struct context_vector {
	size_t *context_map; //map N^2 byte pairs to corresponding frequency.
	float weight;
};

class ContextCradle {
private:
	float learning_rate;
	size_t k; // number of contexts (independent).
	size_t alphabet_size;
	context_vector *context_vectors;
	ringbuffer *rbuf;

public:
	ContextCradle(size_t alphabet_size, float learning_rate, size_t ncontexts);
	size_t feed(size_t obj);
};