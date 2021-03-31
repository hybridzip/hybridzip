R"(

// Required dependencies:
// types.cl

u32 absdiff(u32 x, u32 y) {
	return x > y ? x - y : y - x;
}

u32 min3(u32 x, u32 y, u32 z) {
	u32 m = x;
	if (y < m) {
		m = y;
	}
	
	if (z < m) {
		m = z;
	}
	
	return m;
}


__kernel void paeth_differential16(
	__global u16 *g_input,
	__global u16 *g_output,
	const u64 width,
	const u64 height,
	const u8 bit_depth,
	const u64 n,
	const u64 s
) {
	u64 tid = get_global_id(0);
	if (tid >= n) {
		return;
	}
	
	u64 start_index = tid * s;
	u64 end_index = start_index + s - 1;
	
	if (end_index >= n) {
		end_index = n - 1;
	}
	
	// Now proceed with the paeth differential.
	for (u64 index = start_index; index <= end_index; index++) {
		u16 t = g_input[index];
		u64 x = index % width;
		u64 y = index / width;
		
		if (x > 0 && y > 0) {
			u32 a = g_input[index - 1];
			u32 b = g_input[index - width];
			u32 c = g_input[index - width - 1];
			
			u32 d = absdiff(a + b, c);
			u32 da = absdiff(a, d);
			u32 db = absdiff(b, d);
			u32 dc = absdiff(c, d);
			
			u32 dd = min3(da, db, dc);
			
			if (da == dd) {
				d = a;
			} else if (db == dd) {
				d = b;
			} else {
				d = c;
			}
			
			u16 diff;
			if (t >= d) {
				diff = t - d;
			} else {
				diff = (1 << bit_depth) + t - d;
			}
			
			g_output[index] = diff;
			continue;
		}
		
		if (x > 0 && y == 0) {
			u16 d = g_input[index - 1];
			
			u16 diff;
			if (t >= d) {
				diff = t - d;
			} else {
				diff = (1 << bit_depth) + t - d;
			}
			
			g_output[index] = diff;
			continue;
		}
		
		
		if (x == 0 && y > 0) {
			u16 d = g_input[index - width];
			
			u16 diff;
			if (t >= d) {
				diff = t - d;
			} else {
				diff = (1 << bit_depth) + t - d;
			}
			
			g_output[index] = diff;
			
			continue;
		}
		
		if (x == 0 && y == 0) {
			g_output[index] = t;
		}
	}
}

)"