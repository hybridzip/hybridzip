R"(

#define u8 unsigned char
#define u64 unsigned long int

struct pair {
	u8 x;
	u8 y;
}

pair forward_lift(pair p) {
	pair out;
	out.y = p.y - p.x;
	out.x = p.x + (out.y >> 1);
	
	return out;
}
	
__kernel void rgb_to_ycocg(__global u8 *arr, const u64 n, const u64 s) {
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
		u8 r = arr[index];
		u8 g = arr[index + n];
		u8 b = arr[index + n2];
		
		pair rb;
		rb.x = r;
		rb.y = b;
		
		pair temp_co = forward_lift(rb);
		
		pair gt;
		gt.x = g;
		gt.y = temp_co.x;
		
		pair y_cg = forward_lift(gt);
		
		arr[index] = y_cg.x;
		arr[index + n] = y_cg.y;
		arr[index + n2] = temp_co.y;
		
		if (index == end_index) {
			break;
		}
		
		index++;
	}
}


)"