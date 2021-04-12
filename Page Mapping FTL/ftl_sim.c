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

#include "ftl.h"

void sim_init(void)
{
	s.gc = 0;
	s.host_write = 0;
	s.gc_write = 0;
	srand(0);

	ftl_open();
}

void show_info(void)
{
	printf("Bank: %d\n", N_BANKS);
	printf("Blocks / Bank: %d blocks\n", BLKS_PER_BANK);
	printf("Pages / Block: %d pages\n", PAGES_PER_BLK);
	printf("OP ratio: %d%%\n", OP_RATIO);
	printf("Physical Blocks: %d\n", N_BLOCKS);
	printf("User Blocks: %d\n", N_USER_BLOCKS);
	printf("OP Blocks: %d\n", N_OP_BLOCKS);
	printf("PPNs: %d\n", N_PPNS);
	printf("LPNs: %d\n", N_LPNS);

#ifndef HOT_COLD
	printf("Workload: Random\n");
#else
	printf("Workload: Hot %d / Cold %d\n", HOT_RATIO, COLD_RATIO);
#endif

#ifndef COST_BENEFIT
	printf("FTL: Greedy policy\n");
#else
	printf("FTL: Cost-Benefit policy\n");
#endif

	printf("\n");
}

u32 get_lpn()
{
	long lpn;

#ifdef HOT_COLD
	double prob;

	prob = rand() % 100;
	if (prob < HOT_RATIO) 
	{
	// HOT
	//	printf("HOT\n");
		lpn = rand() % HOT_LPN;
	} 
	else 
	{
	// COLD
	//	printf("COLD\n");
		lpn = HOT_LPN + (rand() % COLD_LPN);
	}

#else
	lpn = rand() % N_LPNS;
#endif

	return lpn;
}

u32 get_data(u32 lpn)
{
	if (lpn % 0xF == 0x0) return 0x00;
	if (lpn % 0xF == 0x1) return 0x11;
	else if (lpn % 0xF == 0x2) return 0x22;
	else if (lpn % 0xF == 0x3) return 0x33;
	else if (lpn % 0xF == 0x4) return 0x44;
	else if (lpn % 0xF == 0x5) return 0x55;
	else if (lpn % 0xF == 0x6) return 0x66;
	else if (lpn % 0xF == 0x7) return 0x77;
	else if (lpn % 0xF == 0x8) return 0x88;
	else if (lpn % 0xF == 0x9) return 0x99;
	else if (lpn % 0xF == 0xa) return 0xaa;
	else if (lpn % 0xF == 0xb) return 0xbb;
	else if (lpn % 0xF == 0xc) return 0xcc;
	else if (lpn % 0xF == 0xd) return 0xdd;
	else if (lpn % 0xF == 0xe) return 0xee;
	else return 0xff;
}

void sim()
{
	u32 lpn;
	u32 write_buffer[SECTORS_PER_PAGE];
	u32 read_buffer[SECTORS_PER_PAGE];

	while (s.host_write < MAX_ITERATION)
	{
		lpn = get_lpn();
		
		memset(write_buffer, get_data(lpn), DATA_SIZE);
		memset(read_buffer, 0, DATA_SIZE);

		ftl_write(lpn, write_buffer);
		ftl_read(lpn, read_buffer);
		
		if (memcmp(write_buffer, read_buffer, DATA_SIZE)) assert(0); 

		s.host_write++;
		if (s.host_write % N_LPNS == 0)
		{
			printf("[Run %d] host %ld, valid page copy %ld, GC# %d, WAF %.2f\n",\
				(int)s.host_write/N_LPNS, s.host_write, s.gc_write, s.gc, (double)(s.host_write+s.gc_write)/(double)s.host_write);
		}

	}
}

void show_stat(void)
{
	printf("\nResults ------\n");
	printf("Host writes: %ld\n", s.host_write);
	printf("GC writes: %ld\n", s.gc_write);
	printf("Number of GCs: %d\n", s.gc);
	printf("Valid pages per GC: %.2f pages\n", (double)s.gc_write / (double)s.gc);
	printf("WAF: %.2f\n", (double)(s.host_write + s.gc_write) / (double)s.host_write);
}

int main(void)
{
	sim_init();
	show_info();
	sim();
	show_stat();
	return 0;
}
