R"(

#define unsigned char u8
#define unsigned short u16
#define unsigned int u32
#define unsigned long int u64

typedef struct __attribute__ ((packed)) _sstpair {
	u32 ls;
	u32 bs;
} sstpair;
	
__kernel void sharingan_dynamic(
	__global u16 *g_input,
	__global sstpair *g_output,
	const u64 n,
	const u64 chunk_width,
	const u64 chunk_height,
	const u64 super_offset,
	const u64 locality_context_order,
	const u64 shift,
	const bool enable_zero_order,
	const u64 width,
	const u64 height,
	const u64 nchannels
) {
	
}



)"