R"(

// Required dependencies:
// types.cl

typedef struct __attribute__ ((packed)) _pair16 {
	u16 x;
	u16 y;
} pair16;

pair16 forward_lift16(pair16 p) {
	pair16 out;
	
	out.y = p.y - p.x;
	out.x = p.x + (out.y >> 1);
	
	return out;
}
	
__kernel void rgb_to_ycocg16(__global u16 *arr, const u64 n, const u64 s) {
	u64 tid = get_global_id(0);
	u64 start_index = tid * s;
	
	if (start_index >= n) {
		return;
	}
	
	u64 end_index = start_index + s - 1;
	
	if (end_index >= n) {
		end_index = n - 1;
	}
	
	u64 n2 = n * 2;
	
	
	u64 index = start_index;
	while (true) {
		u16 r = arr[index];
		u16 g = arr[index + n];
		u16 b = arr[index + n2];
		
		pair16 rb;
		rb.x = r;
		rb.y = b;
		
		pair16 temp_co = forward_lift16(rb);
		
		pair16 gt;
		gt.x = g;
		gt.y = temp_co.x;
		
		pair16 y_cg = forward_lift16(gt);
		
		arr[index] = y_cg.x;
		arr[index + n] = temp_co.y;
		arr[index + n2] = y_cg.y;
		
		if (index == end_index) {
			break;
		}
		
		index++;
	}
}

pair16 reverse_lift16(pair16 p) {
	pair16 out;
	out.x = p.x - (p.y >> 1);
	out.y = out.x + p.y;
	
	return out;
}

__kernel void ycocg_to_rgb16(__global u16 *arr, const u64 n, const u64 s) {
	u64 tid = get_global_id(0);
	u64 start_index = tid * s;
	
	if (start_index >= n) {
		return;
	}
	
	u64 end_index = start_index + s - 1;
	
	if (end_index >= n) {
		end_index = n - 1;
	}
	
	u64 n2 = n * 2;
	
	
	u64 index = start_index;
	while (true) {
		u16 y = arr[index];
		u16 co = arr[index + n];
		u16 cg = arr[index + n2];
		
		pair16 ycg;
		ycg.x = y;
		ycg.y = cg;
		
		pair16 green_temp = reverse_lift16(ycg);
		
		pair16 temp_co;
		temp_co.x = green_temp.y;
		temp_co.y = co;
		
		pair16 rb = reverse_lift16(temp_co);
		
		arr[index] = rb.x;
		arr[index + n] = green_temp.x;
		arr[index + n2] = rb.y;
		
		if (index == end_index) {
			break;
		}
		
		index++;
	}
}

)"