#pragma once
#include <hls_vector.h>
#include <hls_stream.h>
#include <ap_int.h>
#include "ap_axi_sdata.h"
#include "ap_axi_sdata.h"
#include <utils/x_hls_utils.h>
#include "assert.h"
#include "ape_hls/hapecom.hpp"

using namespace hls;

// UF depends on the number of rows of the image.
// the number of pipeline restart paid is roughly given by NumberOfRows/UF + UF
// when Nrows=O(8k), UF = 90
// when Nrows=O(4k), UF = 64
// when Nrows=O(2k), UF = 48
// when Nrows=O(1k), UF = 32
//#define UF 125 //unroll factor for the loop in the function do_filtering

#define UF 64 //unroll factor for the loop in the function do_filtering

#define SIZE (8)  //size of one image component (in bit)
#define SIZEx2 (SIZE*2)
#define SIZEx4 (SIZE*4)
#define MAX_OUT_VALUE (255)  // max value that can be represented with the bits used by the output image
#define MAX_Y 256 //corresponds to 8 bit/component
#define F_MAX_Y 256.0f //corresponds to 8 bit/component

#define N (64)   //size of the input word (in bit)
#define Nx2 (N*2)
#define Nx4 (N*4)
#define PIXEL_IN_INPUT_WORD (N/SIZE)
#define PIXEL_PROCESSED_IN_PARALLEL (PIXEL_IN_INPUT_WORD/2)
typedef ap_uint<N> dt;
typedef ap_uint<4*N> dtx4;
typedef ap_uint<64> dt8;   //8 bytes
typedef ap_uint<128> dt16;   //16 bytes
typedef ap_uint<256> dt32;   //32 bytes
typedef ap_uint<512> dt64;   //64 bytes
#define MAX_LINE_SIZE (6144*3*SIZE/N)  // expressed in number of dt - in the example, 16 bytes. MAX num of column allowed in a color image is 4096
#define MAX_COMPONENT_LINE_SIZE (6144*SIZE/N)  // expressed in number of dt - in the example, 64 bytes. MAX num of column allowed in a color image is 4096
#define INTERNAL_BUFF_SIZE 256  // used in the buffer kernel, to implement double buffering scheme


#define N_ADLER 16
#define ADLER_MOD 65521

// hls::axis<ap_uint<WData>, WUser, WId, WDest>
// hls::axis<ap_int<WData>, WUser, WId, WDest>

// typedef ap_axiu<DWIDTH, 1, 1, 1> trans_pkt;
// needed definitions
//typedef ap_axiu<32, 0, 0, 0> io_stream;
//typedef struct{ //Output AXI-Stream structure
//	ap_uint<32> data; //Four bytes of data
//	ap_uint<4> keep;  //TKEEP Signals for each byte
//	bool last;	   //TLAST AXI-Stream signal
//} io_stream;

typedef ap_axiu<32, 1, 0, 0> lld_stream;
typedef struct{ //Literal, Length/Distance stream structure
	ap_uint<32> data; //Four bytes of data
	ap_uint<4> keep;  //TKEEP Signals for each byte
	bool user;	   //TUSER Signal to identify Literal(0) or Length-Distance pair(1)
	bool last;	   //TLAST AXI-Stream signal
} lld_stream_internal;

typedef ap_axiu<64, 0, 0, 0> io_stream_8B;
typedef struct{ //Output AXI-Stream structure
	ap_uint<8*8> data; //Four bytes of data
	ap_uint<8> keep;  //TKEEP Signals for each byte
	bool last;	   //TLAST AXI-Stream signal
} io_stream_8B_internal;

typedef ap_axiu<128, 0, 0, 0> io_stream_16B;
typedef struct{ //Output AXI-Stream structure
	ap_uint<16*8> data; //Four bytes of data
	ap_uint<16> keep;  //TKEEP Signals for each byte
	bool last;	   //TLAST AXI-Stream signal
} io_stream_16B_internal;

typedef ap_axiu<256, 0, 0, 0> io_stream_32B;
typedef struct{ //Output AXI-Stream structure
	ap_uint<2*16*8> data; //Four bytes of data
	ap_uint<2*16> keep;  //TKEEP Signals for each byte
	bool last;	   //TLAST AXI-Stream signal
} io_stream_32B_internal;


//typedef ap_axiu<384, 32, 0, 0> io_stream_48B_tuser;
typedef struct{ //Output AXI-Stream structure
	ap_uint<3*16*8> data; //48 bytes of data
	ap_uint<2*16> user;  //TKEEP Signals for each byte
	bool last;	   //TLAST AXI-Stream signal
} io_stream_48B_tuser;

typedef struct{ //Distance code table structure
	ap_uint<5>  code;
	ap_uint<15> base;
	ap_uint<4>  bits;
} distance_code;

#define BUFF_SIZE 32768  //from LZ77_Decoder

#define WINDOW_SIZE   16	//Number of boxes in input window
#define WINDOW_BITS   WINDOW_SIZE*8
#define ENC_WIN_BITS  (WINDOW_SIZE-1)*9 + 26 //Bit width of encoded window. Longest possible enc_win is 15 9-bit literals and 1 26-bit LD pair = 161 bits.
#define OUT_STRM_BITS 2*WINDOW_BITS //Output stream bit width, must be power of 2 and larger than encoded window width
#define OUT_WIN_BITS  2*OUT_STRM_BITS //Bit width of output window packer, must be double that of output stream width

typedef ap_uint<ENC_WIN_BITS> t_enc_win;


#define WIN_SIZE     16	      			      //Compare window size. Sliding window is twice this
#define NUM_BANKS	 32					      //Number of dictionary banks

#define BANK_DEPTH   3		  			      //Depth of each dictionary bank
#define BANK_INDEXES (512*NUM_BANKS)//16384   //Total number of hash bank indexes
#define BANK_SIZE	 (BANK_INDEXES/NUM_BANKS) //Number of indexes in each bank
#define MIN_LENGTH   3	      			      //Minimum Deflate match length
#define MAX_DISTANCE 32768   			      //Maximum Deflate match distance

//Dynamic bit width definitions (Only work in C sim)
#ifndef __SYNTHESIS__
#define WIN_SIZE_BITS   (int)(log2(WIN_SIZE-1)+1)     //Number of bits required to store WIN_SIZE values
#define BANK_DEPTH_BITS (int)(log2(BANK_DEPTH-1)+1)   //Number of bits required to store BANK_DEPTH values
#define MATCH_SIZE_BITS (int)(log2(WIN_SIZE)+1)       //Number of bits required to store maximum match length (WIN_SIZE)
#define HASH_BITS       (int)(log2(BANK_INDEXES-1)+1) //Number of hash bits required to index BANK_INDEXES (Complete hash function)
#define NUM_BANKS_BITS  (int)(log2(NUM_BANKS-1)+1)    //Number of bits required to index NUM_BANKS values (Top bits of hash function)
#define BANK_SIZE_BITS  (int)(log2(BANK_SIZE-1)+1)    //Number of bits required to index BANK_SIZE values (Bottom bits of hash function)
#else
#define WIN_SIZE_BITS   4
#define BANK_DEPTH_BITS 2
#define MATCH_SIZE_BITS 5
#define HASH_BITS       14
#define NUM_BANKS_BITS  5
#define BANK_SIZE_BITS  9
#endif

typedef ap_uint<WIN_SIZE_BITS> t_win_size;
typedef ap_uint<BANK_DEPTH_BITS> t_bank_depth;
typedef ap_uint<MATCH_SIZE_BITS> t_match_size;
typedef ap_uint<HASH_BITS> t_hash_size;
typedef ap_uint<NUM_BANKS_BITS> t_num_banks;
typedef ap_uint<NUM_BANKS_BITS+1> t_bank_values; //1 extra bit to include the value NUM_BANKS
typedef ap_uint<BANK_SIZE_BITS> t_bank_size;

// Huffman decoder
#define MAX_LEN_CODES 286 //Max 286 Length/Literal code lengths
#define MIN_LEN_CODES 257 //Min 257 Length/Literal code lengths (256 Literals and 1 End_of_block code)
#define MAX_DIS_CODES 32  //Max 32 Distance code lengths
#define MAX_CL_CODES  19  //Max 19 Code Length code lengths
#define MAX_LEN_LENGTH 15 //Max length of Length/Literal code is 15 bits
#define MAX_DIS_LENGTH 15 //Max length of Distance code is 15 bits
#define MAX_CL_LENGTH  7  //Max length of Code Length code is 7 bits
#define STATIC_MAX_LEN_LENGTH 9 //Max length of Static Length/Literal code is 9 bits
#define STATIC_MIN_LEN_LENGTH 7 //Min length of Static Length/Literal code is 7 bits
#define STATIC_DIS_LENGTH 5 //Static Distance codes are fixed length 5-bit codes
#define MAX_LEN_EXTRA_BITS 5 //Max number of extra bits following a Length code
#define MAX_DIS_EXTRA_BITS 13 //Max number of extra bits following a Distance code

typedef struct{ //Structure for Length symbol table
	ap_uint<3> bits; //Number of extra bits following code. Can be from 0 to 5 for lengths.
	ap_uint<8> base; //Base length value - 3. Lengths 3-258 are encoded in 0-255
} length_symbol_values;

typedef struct{ //Structure for Distance symbol table
	ap_uint<4> bits; //Number of extra bits following code. Can be from 0 to 13 for distances.
	ap_uint<15> base; //Base distance value from 0 to 24577
} distance_symbol_values;

typedef struct{ //Structure for looking up static codes in tables
	ap_uint<8> type; //End-of-block (0110 0000), Literal (0000 0000), or Length/Distance (0001 XXXX) (Where XXXX is number of extra bits)
	ap_uint<8> bits; //Number of bits in code
	ap_uint<16> base; //Base length/distance, or literal value
} static_symbol;


typedef struct{ //Output Array structure
	ap_uint<24> data; //3-byte box for one of the window characters from LZ77 encoder
	ap_uint<2> user;  //2-bit flag for identifying a box as a literal, length, distance, or matched literal
} out_array;

typedef struct{ //Match Structure
	ap_uint<WIN_SIZE*8> string;   //16-byte string, int form
	ap_int<32>			position; //Position of string in history
} match_t;

//16-bit decoder. Given an integer index n, returns a bit string containing n ones.
const ap_uint<16> decoder[17] = {
		/* 0*/ 0b0000000000000000,
		/* 1*/ 0b0000000000000001,
		/* 2*/ 0b0000000000000011,
		/* 3*/ 0b0000000000000111,
		/* 4*/ 0b0000000000001111,
		/* 5*/ 0b0000000000011111,
		/* 6*/ 0b0000000000111111,
		/* 7*/ 0b0000000001111111,
		/* 8*/ 0b0000000011111111,
		/* 9*/ 0b0000000111111111,
		/*10*/ 0b0000001111111111,
		/*11*/ 0b0000011111111111,
		/*12*/ 0b0000111111111111,
		/*13*/ 0b0001111111111111,
		/*14*/ 0b0011111111111111,
		/*15*/ 0b0111111111111111,
		/*16*/ 0b1111111111111111
};

//32-bit decoder. Given an integer index n, returns a bit string containing n ones.
const ap_uint<32> decoder32[33] = {
		/* 0*/ 0b00000000000000000000000000000000,
		/* 1*/ 0b00000000000000000000000000000001,
		/* 2*/ 0b00000000000000000000000000000011,
		/* 3*/ 0b00000000000000000000000000000111,
		/* 4*/ 0b00000000000000000000000000001111,
		/* 5*/ 0b00000000000000000000000000011111,
		/* 6*/ 0b00000000000000000000000000111111,
		/* 7*/ 0b00000000000000000000000001111111,
		/* 8*/ 0b00000000000000000000000011111111,
		/* 9*/ 0b00000000000000000000000111111111,
		/*10*/ 0b00000000000000000000001111111111,
		/*11*/ 0b00000000000000000000011111111111,
		/*12*/ 0b00000000000000000000111111111111,
		/*13*/ 0b00000000000000000001111111111111,
		/*14*/ 0b00000000000000000011111111111111,
		/*15*/ 0b00000000000000000111111111111111,
		/*16*/ 0b00000000000000001111111111111111,
		/*17*/ 0b00000000000000011111111111111111,
		/*18*/ 0b00000000000000111111111111111111,
		/*19*/ 0b00000000000001111111111111111111,
		/*20*/ 0b00000000000011111111111111111111,
		/*21*/ 0b00000000000111111111111111111111,
		/*22*/ 0b00000000001111111111111111111111,
		/*23*/ 0b00000000011111111111111111111111,
		/*24*/ 0b00000000111111111111111111111111,
		/*25*/ 0b00000001111111111111111111111111,
		/*26*/ 0b00000011111111111111111111111111,
		/*27*/ 0b00000111111111111111111111111111,
		/*28*/ 0b00001111111111111111111111111111,
		/*29*/ 0b00011111111111111111111111111111,
		/*30*/ 0b00111111111111111111111111111111,
		/*31*/ 0b01111111111111111111111111111111,
		/*32*/ 0b11111111111111111111111111111111
};

const ap_uint<8> length_code_table[14] = {
		/*Length 3*/   0b1000000, /*7*/ //7-bit length codes with no extra bits
		/*Length 4*/   0b0100000, /*7*/
		/*Length 5*/   0b1100000, /*7*/
		/*Length 6*/   0b0010000, /*7*/
		/*Length 7*/   0b1010000, /*7*/
		/*Length 8*/   0b0110000, /*7*/
		/*Length 9*/   0b1110000, /*7*/
		/*Length 10*/  0b0001000, /*7*/
		/*Length 11*/ 0b01001000, /*8*/ //7-bit length codes with 1 extra bit
		/*Length 12*/ 0b11001000, /*8*/
		/*Length 13*/ 0b00101000, /*8*/
		/*Length 14*/ 0b10101000, /*8*/
		/*Length 15*/ 0b01101000, /*8*/
		/*Length 16*/ 0b11101000  /*8*/
};

//Table for looking up the {Distance Symbol/Code, Base Value, and Extra Bits} of a Distance Value
//Since input distance is 1 less than the actual distance, the base values are pre-decremented by 1 as well
//This way the difference between the two is correct when calculating: extra_value = distance - base_value
const distance_code distance_code_table[512] = {
		{0, 0, 0},  {1, 1, 0},  {2, 2, 0},  {3, 3,	0}, {4, 4,	1}, {4, 4,	1}, {5, 6,	1}, {5, 6,	1},
		{6, 8,	2}, {6, 8,	2}, {6, 8,	2}, {6, 8,	2}, {7, 12, 2}, {7, 12, 2}, {7, 12, 2}, {7, 12, 2},
		{8, 16, 3}, {8, 16, 3}, {8, 16, 3}, {8, 16, 3}, {8, 16, 3}, {8, 16, 3}, {8, 16, 3}, {8, 16, 3},
		{9, 24, 3}, {9, 24, 3}, {9, 24, 3}, {9, 24, 3}, {9, 24, 3}, {9, 24, 3}, {9, 24, 3}, {9, 24, 3},
		{10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4},
		{10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4}, {10, 32, 4},
		{11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4},
		{11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4}, {11, 48, 4},
		{12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5},
		{12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5},
		{12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5},
		{12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5}, {12, 64, 5},
		{13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5},
		{13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5},
		{13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5},
		{13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5}, {13, 96, 5},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6}, {14, 128, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6}, {15, 192, 6},
		{ 0,   0, 0}, { 0,   0, 0}, {16, 256, 7}, {17, 384, 7}, {18, 512, 8}, {18, 512, 8}, {19, 768, 8}, {19, 768, 8},
		{20, 1024,	9}, {20, 1024,	9}, {20, 1024,	9}, {20, 1024,	9}, {21, 1536,	9}, {21, 1536,	9}, {21, 1536,	9}, {21, 1536,	9},
		{22, 2048,	10}, {22, 2048,	10}, {22, 2048,	10}, {22, 2048,	10}, {22, 2048,	10}, {22, 2048,	10}, {22, 2048,	10}, {22, 2048,	10},
		{23, 3072,	10}, {23, 3072,	10}, {23, 3072,	10}, {23, 3072,	10}, {23, 3072,	10}, {23, 3072,	10}, {23, 3072,	10}, {23, 3072,	10},
		{24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11},
		{24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11}, {24, 4096,	11},
		{25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11},
		{25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11}, {25, 6144,	11},
		{26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12},
		{26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12},
		{26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12},
		{26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12}, {26, 8192,	12},
		{27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12},
		{27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12},
		{27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12},
		{27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12}, {27, 12288, 12},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13}, {28, 16384, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13},
		{29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}, {29, 24576, 13}
};

//The permuted order found when reading Code Length code length sequence
const ap_uint<8> permuted_order[MAX_CL_CODES] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

//Each index (symbol) contains that symbols base value and extra bits
//Structure: {Extra bits, Base Value-3}
const length_symbol_values length_symbol_table[29] = { //Length Symbols 257 to 285 are indexed here from 0 to 28
		/* 0*/	{0,	0},   {0, 1},   {0,	2},   {0, 3},
		/* 4*/  {0,	4},   {0, 5},   {0,	6},   {0, 7},
		/* 8*/  {1,	8},   {1, 10},  {1,	12},  {1, 14},
		/*12*/  {2,	16},  {2, 20},  {2,	24},  {2, 28},
		/*16*/  {3,	32},  {3, 40},  {3,	48},  {3, 56},
		/*20*/  {4,	64},  {4, 80},  {4,	96},  {4, 112},
		/*24*/  {5,	128}, {5, 160}, {5, 192}, {5, 224},
		/*28*/  {0,	255}
};

//Structure: {Extra bits, Base value}
const distance_symbol_values distance_symbol_table[30] = {
		/* 0*/  {0, 1},      {0, 2},     {0, 3},     {0, 4},
		/* 4*/	{1,	5},      {1, 7},     {2, 9},     {2, 13},
		/* 8*/	{3,	17},     {3, 25},    {4, 33},    {4, 49},
		/*12*/  {5,	65},     {5, 97},    {6, 129},   {6, 193},
		/*16*/  {7,	257},    {7, 385},   {8, 513},   {8, 769},
		/*20*/	{9,	1025},   {9, 1537},  {10, 2049}, {10, 3073},
		/*24*/	{11, 4097},  {11, 6145}, {12, 8193}, {12, 12289},
		/*28*/  {13, 16385}, {13, 24577}
};

static const static_symbol static_length_table[512] = {
     {96,7,0},{0,8,80},{0,8,16},{20,8,115},{18,7,31},{0,8,112},{0,8,48},
     {0,9,192},{16,7,10},{0,8,96},{0,8,32},{0,9,160},{0,8,0},{0,8,128},
     {0,8,64},{0,9,224},{16,7,6},{0,8,88},{0,8,24},{0,9,144},{19,7,59},
     {0,8,120},{0,8,56},{0,9,208},{17,7,17},{0,8,104},{0,8,40},{0,9,176},
     {0,8,8},{0,8,136},{0,8,72},{0,9,240},{16,7,4},{0,8,84},{0,8,20},
     {21,8,227},{19,7,43},{0,8,116},{0,8,52},{0,9,200},{17,7,13},{0,8,100},
     {0,8,36},{0,9,168},{0,8,4},{0,8,132},{0,8,68},{0,9,232},{16,7,8},
     {0,8,92},{0,8,28},{0,9,152},{20,7,83},{0,8,124},{0,8,60},{0,9,216},
     {18,7,23},{0,8,108},{0,8,44},{0,9,184},{0,8,12},{0,8,140},{0,8,76},
     {0,9,248},{16,7,3},{0,8,82},{0,8,18},{21,8,163},{19,7,35},{0,8,114},
     {0,8,50},{0,9,196},{17,7,11},{0,8,98},{0,8,34},{0,9,164},{0,8,2},
     {0,8,130},{0,8,66},{0,9,228},{16,7,7},{0,8,90},{0,8,26},{0,9,148},
     {20,7,67},{0,8,122},{0,8,58},{0,9,212},{18,7,19},{0,8,106},{0,8,42},
     {0,9,180},{0,8,10},{0,8,138},{0,8,74},{0,9,244},{16,7,5},{0,8,86},
     {0,8,22},{64,8,0},{19,7,51},{0,8,118},{0,8,54},{0,9,204},{17,7,15},
     {0,8,102},{0,8,38},{0,9,172},{0,8,6},{0,8,134},{0,8,70},{0,9,236},
     {16,7,9},{0,8,94},{0,8,30},{0,9,156},{20,7,99},{0,8,126},{0,8,62},
     {0,9,220},{18,7,27},{0,8,110},{0,8,46},{0,9,188},{0,8,14},{0,8,142},
     {0,8,78},{0,9,252},{96,7,0},{0,8,81},{0,8,17},{21,8,131},{18,7,31},
     {0,8,113},{0,8,49},{0,9,194},{16,7,10},{0,8,97},{0,8,33},{0,9,162},
     {0,8,1},{0,8,129},{0,8,65},{0,9,226},{16,7,6},{0,8,89},{0,8,25},
     {0,9,146},{19,7,59},{0,8,121},{0,8,57},{0,9,210},{17,7,17},{0,8,105},
     {0,8,41},{0,9,178},{0,8,9},{0,8,137},{0,8,73},{0,9,242},{16,7,4},
     {0,8,85},{0,8,21},{16,8,258},{19,7,43},{0,8,117},{0,8,53},{0,9,202},
     {17,7,13},{0,8,101},{0,8,37},{0,9,170},{0,8,5},{0,8,133},{0,8,69},
     {0,9,234},{16,7,8},{0,8,93},{0,8,29},{0,9,154},{20,7,83},{0,8,125},
     {0,8,61},{0,9,218},{18,7,23},{0,8,109},{0,8,45},{0,9,186},{0,8,13},
     {0,8,141},{0,8,77},{0,9,250},{16,7,3},{0,8,83},{0,8,19},{21,8,195},
     {19,7,35},{0,8,115},{0,8,51},{0,9,198},{17,7,11},{0,8,99},{0,8,35},
     {0,9,166},{0,8,3},{0,8,131},{0,8,67},{0,9,230},{16,7,7},{0,8,91},
     {0,8,27},{0,9,150},{20,7,67},{0,8,123},{0,8,59},{0,9,214},{18,7,19},
     {0,8,107},{0,8,43},{0,9,182},{0,8,11},{0,8,139},{0,8,75},{0,9,246},
     {16,7,5},{0,8,87},{0,8,23},{64,8,0},{19,7,51},{0,8,119},{0,8,55},
     {0,9,206},{17,7,15},{0,8,103},{0,8,39},{0,9,174},{0,8,7},{0,8,135},
     {0,8,71},{0,9,238},{16,7,9},{0,8,95},{0,8,31},{0,9,158},{20,7,99},
     {0,8,127},{0,8,63},{0,9,222},{18,7,27},{0,8,111},{0,8,47},{0,9,190},
     {0,8,15},{0,8,143},{0,8,79},{0,9,254},{96,7,0},{0,8,80},{0,8,16},
     {20,8,115},{18,7,31},{0,8,112},{0,8,48},{0,9,193},{16,7,10},{0,8,96},
     {0,8,32},{0,9,161},{0,8,0},{0,8,128},{0,8,64},{0,9,225},{16,7,6},
     {0,8,88},{0,8,24},{0,9,145},{19,7,59},{0,8,120},{0,8,56},{0,9,209},
     {17,7,17},{0,8,104},{0,8,40},{0,9,177},{0,8,8},{0,8,136},{0,8,72},
     {0,9,241},{16,7,4},{0,8,84},{0,8,20},{21,8,227},{19,7,43},{0,8,116},
     {0,8,52},{0,9,201},{17,7,13},{0,8,100},{0,8,36},{0,9,169},{0,8,4},
     {0,8,132},{0,8,68},{0,9,233},{16,7,8},{0,8,92},{0,8,28},{0,9,153},
     {20,7,83},{0,8,124},{0,8,60},{0,9,217},{18,7,23},{0,8,108},{0,8,44},
     {0,9,185},{0,8,12},{0,8,140},{0,8,76},{0,9,249},{16,7,3},{0,8,82},
     {0,8,18},{21,8,163},{19,7,35},{0,8,114},{0,8,50},{0,9,197},{17,7,11},
     {0,8,98},{0,8,34},{0,9,165},{0,8,2},{0,8,130},{0,8,66},{0,9,229},
     {16,7,7},{0,8,90},{0,8,26},{0,9,149},{20,7,67},{0,8,122},{0,8,58},
     {0,9,213},{18,7,19},{0,8,106},{0,8,42},{0,9,181},{0,8,10},{0,8,138},
     {0,8,74},{0,9,245},{16,7,5},{0,8,86},{0,8,22},{64,8,0},{19,7,51},
     {0,8,118},{0,8,54},{0,9,205},{17,7,15},{0,8,102},{0,8,38},{0,9,173},
     {0,8,6},{0,8,134},{0,8,70},{0,9,237},{16,7,9},{0,8,94},{0,8,30},
     {0,9,157},{20,7,99},{0,8,126},{0,8,62},{0,9,221},{18,7,27},{0,8,110},
     {0,8,46},{0,9,189},{0,8,14},{0,8,142},{0,8,78},{0,9,253},{96,7,0},
     {0,8,81},{0,8,17},{21,8,131},{18,7,31},{0,8,113},{0,8,49},{0,9,195},
     {16,7,10},{0,8,97},{0,8,33},{0,9,163},{0,8,1},{0,8,129},{0,8,65},
     {0,9,227},{16,7,6},{0,8,89},{0,8,25},{0,9,147},{19,7,59},{0,8,121},
     {0,8,57},{0,9,211},{17,7,17},{0,8,105},{0,8,41},{0,9,179},{0,8,9},
     {0,8,137},{0,8,73},{0,9,243},{16,7,4},{0,8,85},{0,8,21},{16,8,258},
     {19,7,43},{0,8,117},{0,8,53},{0,9,203},{17,7,13},{0,8,101},{0,8,37},
     {0,9,171},{0,8,5},{0,8,133},{0,8,69},{0,9,235},{16,7,8},{0,8,93},
     {0,8,29},{0,9,155},{20,7,83},{0,8,125},{0,8,61},{0,9,219},{18,7,23},
     {0,8,109},{0,8,45},{0,9,187},{0,8,13},{0,8,141},{0,8,77},{0,9,251},
     {16,7,3},{0,8,83},{0,8,19},{21,8,195},{19,7,35},{0,8,115},{0,8,51},
     {0,9,199},{17,7,11},{0,8,99},{0,8,35},{0,9,167},{0,8,3},{0,8,131},
     {0,8,67},{0,9,231},{16,7,7},{0,8,91},{0,8,27},{0,9,151},{20,7,67},
     {0,8,123},{0,8,59},{0,9,215},{18,7,19},{0,8,107},{0,8,43},{0,9,183},
     {0,8,11},{0,8,139},{0,8,75},{0,9,247},{16,7,5},{0,8,87},{0,8,23},
     {64,8,0},{19,7,51},{0,8,119},{0,8,55},{0,9,207},{17,7,15},{0,8,103},
     {0,8,39},{0,9,175},{0,8,7},{0,8,135},{0,8,71},{0,9,239},{16,7,9},
     {0,8,95},{0,8,31},{0,9,159},{20,7,99},{0,8,127},{0,8,63},{0,9,223},
     {18,7,27},{0,8,111},{0,8,47},{0,9,191},{0,8,15},{0,8,143},{0,8,79},
     {0,9,255}
 };

 static const static_symbol static_distance_table[32] = {
     {16,5,1},{23,5,257},{19,5,17},{27,5,4097},{17,5,5},{25,5,1025},
     {21,5,65},{29,5,16385},{16,5,3},{24,5,513},{20,5,33},{28,5,8193},
     {18,5,9},{26,5,2049},{22,5,129},{64,5,0},{16,5,2},{23,5,385},
     {19,5,25},{27,5,6145},{17,5,7},{25,5,1537},{21,5,97},{29,5,24577},
     {16,5,4},{24,5,769},{20,5,49},{28,5,12289},{18,5,13},{26,5,3073},
     {22,5,193},{64,5,0}
 };
