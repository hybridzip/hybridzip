R"(

#define unsigned char u8
#define unsigned short u16
#define unsigned int u32
#define unsigned long int u64
#define HZRANS64_MUL_FACTOR 16776960

typedef struct __attribute__ ((packed)) _sstpair {
	u32 ls;
	u32 bs;
} sstpair;


void hzrans64_create_ftable_nf(u64 *tbl, u64 *res) {
	u64 sum = 256;
	u64 ssum = 0;
	
	for (u16 i = 0; i < 256; i++) {
		sum += tbl[i];
	}
	
	for (u16 i = 0; i < 256; i++) {
		u64 value = 1 + (tbl[i] + 1) * HZRANS64_MUL_FACTOR;
		ssum += value - 1;
		res[i] = value;
	}
	
	ssum = mul_factor - ssum;
	for (u64 i = 0; ssum > 0; i = (i + 1) & 0xff, ssum--) {
		res[i]++;
	}
}

sstpair hzrans64_add_to_seq(u64 *tbl, u64 symbol) {
	sstpair p;
	
	p.ls = symbol;
	u64 bs = 0;
	
	for (u16 i = 0; i < symbol; i++) {
		bs += tbl[i];
	}
	
	p.bs = bs;
	return p;
}

	
__kernel void sharingan_dynamic(
	__global u16 *g_input,
	__global sstpair *g_output,
	const u64 n,
	const u64 chunk_width,
	const u64 chunk_height,
	const u64 channel_index,
	const u64 locality_context_order,
	const u64 shift,
	const bool enable_zero_order,
	const u64 width,
	const u64 height,
	const u64 nchannels,
	const u64 learning_rate
) {
	u64 tid = get_global_id(0);
	u64 partial_offset_y = tid / width;
	u64 partial_offset_x = tid % width;
	
	u64 offset_y = partial_offset_y * chunk_height;
	u64 offset_x = partial_offset_x * chunk_width;
	
	if (offset_x >= width || offset_y >= height) {
		return;
	}
	
	u64 channel_offset = channel_index * width * height;
	
	u64 chunk_y_end = chunk_height;
	u64 chunk_x_end = chunk_width;
	
	if (height - offset_y < chunk_height) {
		chunk_y_end = height - offset_y;
	}
	
	if (width - offset_x < chunk_width) {
		chunk_x_end = width - offset_x;
	}
	
	u64 super_chunk_offset = channel_offset + offset_y * width + offset_x;
	u64 ls_arr[256] = {0};	
	sstpair p;
	
	if (enable_zero_order) {
		u64 f_dist[256] = {0};
		
		for (u64 y = 0; y < chunk_y_end; y++) {
			u64 super_offset = super_chunk_offset + y * width;
			
			for (u64 x = 0; x < chunk_x_end; x++) {
				u64 index = super_offset + x;
				u64 curr_symbol = g_input[index] >> shift;
				
				hzrans64_create_ftable_nf(f_dist, ls_arr);
				p = hzrans64_add_to_seq(ls_arr, curr_symbol);
				
				f_dist[curr_symbol] += learning_rate;
				
				g_output[index] = p;
			}
		}
	} else {
		u64 f_map[256][256] = {0};
		
		for (u64 y = 0; y < chunk_y_end; y++) {
			u64 super_offset = super_chunk_offset + y * width;
			
			for (u64 x = 0; x < chunk_x_end; x++) {
				u64 index = super_offset + x;
				u64 curr_symbol = g_input[index] >> shift;
				
				if (x > 0) {
					u64 left_symbol = (g_input[index - 1] >> shift) & 0xff;

					hzrans64_create_ftable_nf(f_map[left_symbol], ls_arr);
					p = hzrans64_add_to_seq(ls_arr, curr_symbol);

					f_map[left_symbol][curr_symbol] += learning_rate;

					if (y > 0) {
						if (locality_context_order > 1) {
							u64 up_symbol = (g_input[index - width] >> shift) & 0xff;
							f_map[up_symbol][curr_symbol] += learning_rate;
						}

						if (locality_context_order > 2) {
							u64 diag_symbol = (g_input[index - width - 1] >> shift) & 0xff;
							f_map[diag_symbol][curr_symbol] += (learning_rate >> 1);
						}
					}
				} else if (y > 0) {
					if (locality_context_order > 1) {
						u64 up_symbol = (g_input[index - width] >> shift) & 0xff;

						hzrans64_create_ftable_nf(f_map[up_symbol], ls_arr);
						p = hzrans64_add_to_seq(ls_arr, curr_symbol);

						f_map[up_symbol][curr_symbol] += learning_rate;
					} else {
						p.ls = 65536;
						p.bs = curr_symbol << 16;
					}
				} else {
					p.ls = 65536;
					p.bs = curr_symbol << 16;
				}
				
				g_output[index] = p;
			}
		}
	}
}

)"