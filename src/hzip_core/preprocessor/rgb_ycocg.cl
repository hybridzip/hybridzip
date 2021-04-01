R"(

// Required dependencies:
// common/types.cl
// common/extended_types.cl

u16_pair u16x_forward_lift(u64 mask, u16_pair p) {
	u16_pair out;
	u16 x = p.first;
	u16 y = p.second;
	
	x &= mask;
	y &= mask;
	
	out.second = (y - x) & mask;
	out.first = (x + (out.second >> 1)) & mask;
	
	return out;
}
	
__kernel void u16x_rgb_to_ycocg(__global u16 *arr, const u64 n, const u64 s, const u64 mask) {
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
		
		u16_pair rb;
		rb.first = r;
		rb.second = b;
		
		u16_pair temp_co = u16x_forward_lift(mask, rb);
		
		u16_pair gt;
		gt.first = g;
		gt.second = temp_co.first;
		
		u16_pair y_cg = u16x_forward_lift(mask, gt);
		
		arr[index] = y_cg.first;
		arr[index + n] = temp_co.second;
		arr[index + n2] = y_cg.second;
		
		if (index == end_index) {
			break;
		}
		
		index++;
	}
}

u16_pair u16x_reverse_lift(u64 mask, u16_pair p) {
	u16_pair out;
	u16 x = p.first;
	u16 y = p.second;
	
	x &= mask;
	y &= mask;
	
	out.first = (x - (y >> 1)) & mask;
	out.second = (out.first + y) & mask;
	
	return out;
}

__kernel void u16x_ycocg_to_rgb(__global u16 *arr, const u64 n, const u64 s, const u64 mask) {
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
		
		u16_pair ycg;
		ycg.first = y;
		ycg.second = cg;
		
		u16_pair green_temp = u16x_reverse_lift(mask, ycg);
		
		u16_pair temp_co;
		temp_co.first = green_temp.second;
		temp_co.second = co;
		
		u16_pair rb = u16x_reverse_lift(mask, temp_co);
		
		arr[index] = rb.first;
		arr[index + n] = green_temp.first;
		arr[index + n2] = rb.second;
		
		if (index == end_index) {
			break;
		}
		
		index++;
	}
}

)"