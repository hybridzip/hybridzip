R"(

#define u8 unsigned char
#define u64 unsigned long int

typedef struct __attribute__ ((packed)) _pair {
	u8 x;
	u8 y;
} pair;

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
		arr[index + n] = temp_co.y;
		arr[index + n2] = y_cg.y;
		
		if (index == end_index) {
			break;
		}
		
		index++;
	}
}

pair reverse_lift(pair p) {
	pair out;
	out.x = p.x - (p.y >> 1);
	out.y = out.x + p.y;
	
	return out;
}

__kernel void ycocg_to_rgb(__global u8 *arr, const u64 n, const u64 s) {
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
		u8 y = arr[index];
		u8 co = arr[index + n];
		u8 cg = arr[index + n2];
		
		pair ycg;
		ycg.x = y;
		ycg.y = cg;
		
		pair green_temp = reverse_lift(ycg);
		
		pair temp_co;
		temp_co.x = green_temp.y;
		temp_co.y = co;
		
		pair rb = reverse_lift(temp_co);
		
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