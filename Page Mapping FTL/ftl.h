//----------------------------------------------------------
//
// Lab #4 : Page Mapping FTL Simulator
// 	- Embedded System Design, ICE3028 (Fall, 2019)
//
// Oct. 10, 2019.
//
// Junho Lee, Somm Kim
// Dongkun Shin (dongkun@skku.edu)
// Embedded Systems Laboratory
// Sungkyunkwan University
// http://nyx.skku.ac.kr
//
//---------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "nand.h"

#define N_BANKS						2
#define BLKS_PER_BANK				32
#define PAGES_PER_BLK				32
#define SECTORS_PER_PAGE			(DATA_SIZE / sizeof(u32))

#define N_GC_BLOCKS					1

/* Per Bank */
#define OP_RATIO					7
#define N_PPNS_PB					(BLKS_PER_BANK * PAGES_PER_BLK)
#define N_USER_BLOCKS_PB       		((BLKS_PER_BANK - N_GC_BLOCKS) * 100 / (100 + OP_RATIO))
#define N_OP_BLOCKS_PB				(BLKS_PER_BANK - N_USER_BLOCKS_PB)
#define N_LPNS_PB					(N_USER_BLOCKS_PB * PAGES_PER_BLK)

#define N_PPNS						(N_PPNS_PB * N_BANKS)
#define N_BLOCKS					(BLKS_PER_BANK * N_BANKS)
#define N_USER_BLOCKS				(N_USER_BLOCKS_PB * N_BANKS)
#define N_OP_BLOCKS					(N_OP_BLOCKS_PB * N_BANKS)

#define N_LPNS						(N_LPNS_PB * N_BANKS)

#define N_RUNS						200
#define MAX_ITERATION				(N_LPNS * N_RUNS)

#define CHECK_VPAGE(vpn)			assert((vpn >= 0 && vpn < N_PPNS_PB) || vpn == MAX)
#define CHECK_LPAGE(lpn)			assert((lpn) < N_LPNS)

#define COST_BENEFIT
//#define HOT_COLD

#define HOT_RATIO				90
#define COLD_RATIO				(100 - HOT_RATIO)

#define HOT_LPN_RATIO 			(100 - HOT_RATIO)
#define COLD_LPN_RATIO			(100 - HOT_LPN_RATIO)

#define HOT_LPN 				((N_LPNS * HOT_LPN_RATIO) / 100)
#define COLD_LPN				((N_LPNS * COLD_LPN_RATIO) / 100)

struct ftl_stat {
	int gc;
	long host_write;
	long gc_write;
} s;

void ftl_open();
void ftl_write(u32 lpn, u32 *write_buffer);
void ftl_read(u32 lpn, u32 *read_buffer);
