// SPDX-FileCopyrightText: 2022 INFN APE Lab - Sezione di Roma
// SPDX-License-Identifier: EUPL-1.2

#ifndef __APENET_PACKET_H__
#define __APENET_PACKET_H__

extern "C" {

typedef union {
	struct __attribute__((packed)) {
		unsigned long virt_chan      :  5;
		unsigned long proc_id        : 16;
		unsigned long dest_x         :  6;
		unsigned long dest_y         :  5;
		unsigned long dest_z         :  5;
		unsigned long intra_dest     :  4;
		unsigned long reserved       :  1;
		unsigned long out_of_lattice :  1;
		unsigned long packet_type    :  5;
		unsigned long packet_size    : 14;
		unsigned long dest_addr      : 48;
		unsigned long num_of_hops    : 10;
		unsigned long edac           :  8;
	} s;
	uint32_t l[4];
	uint64_t u[2];
} apenet_header_t;

typedef union {
	uint32_t l[4];
	uint64_t u[2];
} apenet_footer_t;

}

#endif
