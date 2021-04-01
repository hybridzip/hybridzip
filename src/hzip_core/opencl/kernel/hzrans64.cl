R"(

typedef struct __hzrans64_t {
	u64 *ftable;
	u64 scale;
	u64 size;
	u64 ls;
	u64 bs;
} hzrans64_t;


typedef struct __attribute__ ((packed)) __hzrans64_tp {
	u32 ls;
	u32 bs;
} hzrans64_tp;

void hzrans64_create_ftable_nf(hzrans64_t *state, u64 *freq) {
	u64 *ftable = state->ftable;
	u64 sum = state->size;
	u64 ssum = 0;
	
	for (u16 i = 0; i < state->size; i++) {
		sum += freq[i];
	}
	
	u64 mul_factor = (1ul << state->scale) - state->size;
	
	for (u16 i = 0; i < state->size; i++) {
		u64 value = 1 + (freq[i] + 1) * mul_factor / sum;
		ssum += value - 1;
		ftable[i] = value;
	}
	
	ssum = mul_factor - ssum;
	
	for (u64 i = 0; ssum > 0; i = (i + 1) % state->size, ssum--) {
		ftable[i]++;
	}
}

void hzrans64_add_to_seq(hzrans64_t *state, u64 symbol) {
	u64 *ftable = state->ftable;
	state->ls = ftable[symbol];
	u64 bs = 0;
	
	for (u64 i = 0; i < symbol; i++) {
		bs += ftable[i];
	}
	
	state->bs = bs;
}


)"