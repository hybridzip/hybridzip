R"(

// Required dependencies:
// common/types.cl

typedef struct __attribute__ ((packed)) __u8_pair {
	u8 first;
	u8 second;
} u8_pair;

typedef struct __attribute__ ((packed)) __u16_pair {
	u16 first;
	u16 second;
} u16_pair;

)"