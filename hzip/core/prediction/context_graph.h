#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "../../other/platform.h"
#include "../entropy-coders/fgk.h"
#include "../../bitio/bitio.h"


namespace predictors {
	typedef struct ContextNode {
		FGK::FGKTree tree;
		HZIP_SIZE_T freq[0x100];
	}cnode_t;

	class SingleContextGraph {
	private:
		HZIP_SIZE_T prev;
		cnode_t univ, *pivots;
		BitIO *bitio;

	public:
		SingleContextGraph();
		~SingleContextGraph();
		void setBitIO(BitIO *bitio);
		void feed(HZIP_UINT node);

	};


}