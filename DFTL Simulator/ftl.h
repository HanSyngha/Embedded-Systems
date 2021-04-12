//----------------------------------------------------------
//
// Project #1 : DFTL Simulator
// 	- Embedded System Design, ICE3028 (Fall, 2019)
//
// Oct. 31, 2019.
//
// TA: Junho Lee, Somm Kim
// Prof: Dongkun Shin
// Embedded Software Laboratory
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

//#define NO_CACHE

#define N_BANKS						4
#define BLKS_PER_BANK				128
#define PAGES_PER_BLK				128
#define SECTORS_PER_PAGE			(DATA_SIZE / sizeof(u32))

#define OP_RATIO					20

#define MAP_OP_RATIO				60
#define USER_OP_RATIO				(100 - MAP_OP_RATIO)

#define N_GC_THRESHOLD				1
#define N_MAP_GC_THRESHOLD			1

#define MAP_ENTRY_SIZE				(sizeof(u32))
#define N_MAP_ENTRIES_PER_PAGE		(DATA_SIZE / MAP_ENTRY_SIZE)

/* Per Bank */
#define CMT_RATIO					5
#define CMT_SIZE_PB					(BLKS_PER_BANK * PAGES_PER_BLK * sizeof(u32) * CMT_RATIO / 100) // 5% of total map table

#ifdef NO_CACHE
#define N_CACHED_MAP_PAGE_PB		(1)
#else
#define N_CACHED_MAP_PAGE_PB		((const int)(CMT_SIZE_PB / (MAP_ENTRY_SIZE * N_MAP_ENTRIES_PER_PAGE)))
#endif

#define N_PPNS_PB					(BLKS_PER_BANK * PAGES_PER_BLK)
#define N_MAP_PAGES_PB				(N_PPNS_PB / N_MAP_ENTRIES_PER_PAGE)
#define N_MAP_BLOCKS_PB				(N_MAP_PAGES_PB / PAGES_PER_BLK)

#define N_USER_BLOCKS_PB			(((BLKS_PER_BANK - N_MAP_BLOCKS_PB) * 100) / (100 + OP_RATIO))
#define N_OP_BLOCKS_PB				(BLKS_PER_BANK - N_MAP_BLOCKS_PB - N_USER_BLOCKS_PB)
#define N_OP_USER_BLOCKS_PB			(N_OP_BLOCKS_PB * USER_OP_RATIO / 100)
#define N_OP_MAP_BLOCKS_PB			(N_OP_BLOCKS_PB - N_OP_USER_BLOCKS_PB)

#define N_LPNS_PB					((N_USER_BLOCKS_PB) * PAGES_PER_BLK)

#define N_PPNS						(N_PPNS_PB * N_BANKS)
#define N_BLOCKS					(BLKS_PER_BANK * N_BANKS)
#define N_MAP_BLOCKS				(N_MAP_BLOCKS_PB * N_BANKS)
#define N_USER_BLOCKS				(N_USER_BLOCKS_PB * N_BANKS)
#define N_OP_BLOCKS					(N_OP_BLOCKS_PB * N_BANKS)

#define N_LPNS						(N_LPNS_PB * N_BANKS)
#define N_LBAS						(N_LPNS * SECTORS_PER_PAGE)

#define N_RUNS						10
#define MAX_ITERATION				(N_LPNS * N_RUNS)

#define CHECK_VPAGE(vpn)			assert((vpn >= 0 && vpn < N_PPNS_PB) || vpn == MAX)
#define CHECK_LPAGE(lpn)			assert(lpn < N_LPNS)

struct ftl_stat {
	int gc;
	long host_write;
	long ftl_write;
	long gc_write;
	long map_write;
	long map_gc;
	long map_gc_write;
	long cache_hit;
	long cache_miss;
} s;

void ftl_open();
void ftl_write(u32 lba, u32 num_sectors, u32 *write_buffer);
void ftl_read(u32 lba, u32 num_sectors, u32 *read_buffer);
