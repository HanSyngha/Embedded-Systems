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

#include "ftl.h"

int iter = 0;
int max_sectors = 32;

void sim_init(void)
{
	s.gc = 0;
	s.host_write = 0;
	s.ftl_write = 0;
	s.gc_write = 0;

	s.map_write = 0;
	s.map_gc = 0;
	s.map_gc_write = 0;

	s.cache_hit = 0;
	s.cache_miss = 0;

	srand(0);

	ftl_open();
}

void show_info(void)
{
	printf("Bank: %d\n", N_BANKS);
	printf("Blocks / Bank: %d blocks\n", BLKS_PER_BANK);
	printf("Pages / Block: %d pages\n", PAGES_PER_BLK);
	printf("OP ratio: %d%%\n", OP_RATIO);
	printf("Physical Blocks: %ld\n", (long)N_BLOCKS);
	printf("User Blocks: %ld\n", (long)N_USER_BLOCKS);
	printf("Map blocks: %ld\n", (long)N_MAP_BLOCKS);
	printf("OP User Blocks: %ld\n", (long)(N_OP_USER_BLOCKS_PB*N_BANKS));
	printf("OP Map Blocks: %ld\n", (long)(N_OP_MAP_BLOCKS_PB*N_BANKS));
	printf("PPNs: %ld\n", (long)N_PPNS);
	printf("LPNs: %ld\n", (long)N_LPNS);

	printf("Max Sectors: %d\n", max_sectors);
	
	printf("MAP_OP_RATIO: %d\n", MAP_OP_RATIO);
	printf("CMT Size: %ld\n", (long)(CMT_SIZE_PB * N_BANKS));

	printf("\n");
}

long get_lba()
{
	long lba;
	lba = rand() % N_LBAS;

	return lba;
}

long get_sector_cnt(long lba)
{
	if (N_LBAS - lba < max_sectors) {
		return (rand() % (N_LBAS - lba)) + 1;
	}

	return (rand() % max_sectors) + 1;
}

long get_data(long lba)
{
	if (lba % 0xF == 0x0) return 0x00;
	else if (lba % 0xF == 0x1) return 0x11;
	else if (lba % 0xF == 0x2) return 0x22;
	else if (lba % 0xF == 0x3) return 0x33;
	else if (lba % 0xF == 0x4) return 0x44;
	else if (lba % 0xF == 0x5) return 0x55;
	else if (lba % 0xF == 0x6) return 0x66;
	else if (lba % 0xF == 0x7) return 0x77;
	else if (lba % 0xF == 0x8) return 0x88;
	else if (lba % 0xF == 0x9) return 0x99;
	else if (lba % 0xF == 0xa) return 0xaa;
	else if (lba % 0xF == 0xb) return 0xbb;
	else if (lba % 0xF == 0xc) return 0xcc;
	else if (lba % 0xF == 0xd) return 0xdd;
	else if (lba % 0xF == 0xe) return 0xee;
	else return 0xff;
}

void sim()
{
	long lba, num_sectors, i;
	u32 *write_buffer;
	u32 *read_buffer;

	while (iter < MAX_ITERATION)
	{
		lba = get_lba();
		num_sectors = get_sector_cnt(lba);

		write_buffer = (u32 *)calloc(num_sectors, sizeof(u32));
		read_buffer = (u32 *)calloc(num_sectors, sizeof(u32));

		for (i = 0; i < num_sectors; i++) {
			memset(write_buffer + i, get_data(lba + i), sizeof(u32));
		}

		ftl_write(lba, num_sectors, write_buffer);
		ftl_read(lba, num_sectors, read_buffer);

		if (memcmp(write_buffer, read_buffer, num_sectors * sizeof(u32))) assert(0); 

		s.host_write += num_sectors;
		iter++;
		if (iter % N_LPNS == 0) {
			printf("[Run %ld] host %ld, ftl %ld, valid page copy %ld, GC# %ld, \
hit %ld, miss %ld, WAF %.2f\n", \
(long)(iter/N_LPNS), (long)s.host_write, (long)s.ftl_write, (long)((s.gc_write + s.map_gc_write) / SECTORS_PER_PAGE), \
(long)(s.gc + s.map_gc), (long)s.cache_hit, (long)s.cache_miss, \
(double)(s.ftl_write+s.gc_write+s.map_write+s.map_gc_write)/(double)s.host_write);
		}

		free(write_buffer);
		free(read_buffer);
	}
}

void show_stat(void)
{
	printf("\nResults ------\n");
	printf("Host write sectors: %ld\n", s.host_write);
	printf("FTL write sectors: %ld\n", s.ftl_write);
	printf("GC write sectors: %ld\n", s.gc_write);

	printf("Number of GCs: %d\n", s.gc);
	printf("Valid pages per GC: %.2f pages\n", (double)s.gc_write / SECTORS_PER_PAGE / (double)s.gc);

	printf("Map write sectors: %ld\n", s.map_write);
	printf("Map GC write sectors: %ld\n", s.map_gc_write);

	printf("Number of Map GCs: %ld\n", s.map_gc);
	printf("Valid pages per Map GC: %.2f pages\n", (double)s.map_gc_write / SECTORS_PER_PAGE / (double)s.map_gc);

	printf("Cache hit: %ld, Cache miss: %ld, hit ratio: %.2f\n", (long)s.cache_hit, (long)s.cache_miss, (double)s.cache_hit / (double)(s.cache_hit + s.cache_miss));	

	printf("WAF: %.2f\n", (double)(s.ftl_write + s.gc_write + s.map_write + s.map_gc_write) / (double)s.host_write);
}

int main(void)
{
	sim_init();
	show_info();
	sim();
	show_stat();
	return 0;
}
