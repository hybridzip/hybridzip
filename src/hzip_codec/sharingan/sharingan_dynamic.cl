R"(

// Required dependencies:
// common/types.cl
// kernel/hzrans64.cl
	
__kernel void sharingan_dynamic(
	__global u16 *g_input,
	__global hzrans64_tp *g_output,
	const u64 n,
	const u64 chunk_width,
	const u64 chunk_height,
	const u8 locality_context_order,
	const u8 shift,
	const u8 enable_zero_order,
	const u64 width,
	const u64 height,
	const u64 learning_rate,
	__global u64 *g_freq_map
) {	
	u64 tid = get_global_id(0);
	
	u64 x_chunks = (width / chunk_width) + (width % chunk_width != 0);
	u64 partial_offset_x = tid % x_chunks;
	u64 partial_offset_y = tid / x_chunks;
	
	u64 offset_x = partial_offset_x * chunk_width;
	u64 offset_y = partial_offset_y * chunk_height;
	
	if (offset_y >= height) {
		return;
	}
	
	// Initialize hzrans64 state
	hzrans64_t state;
	state.scale = 24;
	state.size = 256;
	
	u64 hzrans64_state_ftable[256] = {0};
	state.ftable = hzrans64_state_ftable;
	
	u64 chunk_y_end = chunk_height;
	u64 chunk_x_end = chunk_width;
	
	if (height - offset_y < chunk_height) {
		chunk_y_end = height - offset_y;
	}
	
	if (width - offset_x < chunk_width) {
		chunk_x_end = width - offset_x;
	}
	
	u64 super_chunk_offset = offset_y * width + offset_x;
	
	hzrans64_tp p;
	
	if (enable_zero_order) {
		u64 *f_dist = g_freq_map + (tid << 8);
		
		for (u64 y = 0; y < chunk_y_end; y++) {
			u64 super_offset = super_chunk_offset + y * width;
			
			for (u64 x = 0; x < chunk_x_end; x++) {
				u64 index = super_offset + x;
				
				u64 curr_symbol = (g_input[index] >> shift) & 0xff;
				
				hzrans64_create_ftable_nf(&state, f_dist);
				hzrans64_add_to_seq(&state, curr_symbol);
				
				p.ls = state.ls;
				p.bs = state.bs;
				
				f_dist[curr_symbol] += learning_rate;
				
				g_output[index] = p;
			}
		}
	} else {
		u64 *f_map = g_freq_map + (tid << 16);
		
		for (u64 y = 0; y < chunk_y_end; y++) {
			u64 super_offset = super_chunk_offset + y * width;
			
			for (u64 x = 0; x < chunk_x_end; x++) {
				u64 index = super_offset + x;
				u64 curr_symbol = (g_input[index] >> shift) & 0xff;
				
				if (x > 0) {
					u64 left_symbol = (g_input[index - 1] >> shift) & 0xff;

					hzrans64_create_ftable_nf(&state, f_map + (left_symbol << 8));
					hzrans64_add_to_seq(&state, curr_symbol);

					p.ls = state.ls;
					p.bs = state.bs;

					f_map[(left_symbol << 8) + curr_symbol] += learning_rate;

					if (y > 0) {
						if (locality_context_order > 1) {
							u64 up_symbol = (g_input[index - width] >> shift) & 0xff;
							f_map[(up_symbol << 8) + curr_symbol] += learning_rate;
						}

						if (locality_context_order > 2) {
							u64 diag_symbol = (g_input[index - width - 1] >> shift) & 0xff;
							f_map[(diag_symbol << 8) + curr_symbol] += (learning_rate >> 1);
						}
					}
				} else if (y > 0) {
					if (locality_context_order > 1) {
						u64 up_symbol = (g_input[index - width] >> shift) & 0xff;

						hzrans64_create_ftable_nf(&state, f_map + (up_symbol << 8));
						hzrans64_add_to_seq(&state, curr_symbol);

						p.ls = state.ls;
						p.bs = state.bs;

						f_map[(up_symbol << 8) + curr_symbol] += learning_rate;
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