//----------------------------------------------------------
//
// Lab #6 : Multi-Streamed SSD Simulator
// 	- Embedded System Design, ICE3028 (Fall, 2019)
//
// Nov. 07, 2019.
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

void sim_init (void)
{
  	s.gc = 0;
   	s.host_write = 0;
   	s.ftl_write = 0;
   	s.gc_write = 0;
	srand (0);

	ftl_open();
}

void show_info (void)
{
	printf ("Multi-Stream SSD\n\n");

	printf("Bank: %d\n", N_BANKS);
	printf("Blocks / Bank: %d blocks\n", BLKS_PER_BANK);
	printf("Pages / Block: %d pages\n", PAGES_PER_BLK);
	printf("OP ratio: %d%%\n", OP_RATIO);
	printf("Physical Blocks: %d\n", N_BLOCKS);
	printf("User Blocks: %d\n", N_USER_BLOCKS);
	printf("OP Blocks: %d\n", N_OP_BLOCKS);
	printf("PPNS: %d\n", N_PPNS);
	printf("LPNS: %d\n", N_LPNS);

	printf("Workload: Multi Hot/Cold\n");
	
	printf("Max Sectors: %d\n", max_sectors);

	printf("\n");
}

long get_lba(int *streamid)
{
	long lba;

	double prob = random() % 100;
	if (prob < HOT_RATIO)
	{
		lba = random () % HOT_LBA;
		*streamid = HOT_STREAMID;
	}
	else if (prob < HOT_RATIO + WARM_RATIO)
	{
		lba = HOT_LBA + (random () % WARM_LBA); 
		*streamid = WARM_STREAMID;
	}
	else 
	{
		lba = HOT_LBA + WARM_LBA + (random () % COLD_LBA);
		*streamid = COLD_STREAMID;
	}

	return lba;
}

long get_sector_cnt(long lba)
{
	int lba_limit;

	if (lba < HOT_LBA) lba_limit = HOT_LBA;
	else if (lba < HOT_LBA + WARM_LBA) lba_limit = HOT_LBA + WARM_LBA;
	else lba_limit = N_LBAS;

	if (lba_limit - lba < max_sectors) {
		return (rand() % (lba_limit - lba)) + 1;
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
		int streamid;
		lba = get_lba(&streamid);

		num_sectors = get_sector_cnt(lba);

		write_buffer = (u32 *)calloc(num_sectors, sizeof(u32));
		read_buffer = (u32 *)calloc(num_sectors, sizeof(u32));

		for (i = 0; i < num_sectors; i++) {
			memset(write_buffer + i, get_data(lba + i), sizeof(u32));
		}

		ftl_write(streamid, lba, num_sectors, write_buffer);
		ftl_read(lba, num_sectors, read_buffer);

		if (memcmp(write_buffer, read_buffer, num_sectors * sizeof(u32))) assert(0);

		s.host_write += num_sectors;
		iter++;
		if (iter % N_LPNS == 0) {
			printf("[Run %d] host %ld, ftl %ld, valid page copy %ld, GC# %d, WAF %.2f\n", \
				(int)iter/N_LPNS, s.host_write, s.ftl_write, s.gc_write / SECTORS_PER_PAGE, s.gc, (double)(s.ftl_write+s.gc_write)/(double)s.host_write);
		}

		free(write_buffer);
		free(read_buffer);
	}
}

void show_stat (void)
{
	printf ("\nResults ------\n");
	printf ("Host write sectors: %ld\n", s.host_write);
	printf ("FTL write sectors: %ld\n", s.ftl_write);
	printf ("GC write sectors: %ld\n", s.gc_write);
	printf ("Number of GCs: %d\n", s.gc);
	printf ("Valid pages per GC: %.2f pages\n", (double)s.gc_write / SECTORS_PER_PAGE / (double)s.gc);
	printf ("WAF: %.2f\n", (double)(s.ftl_write + s.gc_write) / (double)s.host_write);
}

int main (void)
{
	sim_init ();
	show_info ();
	sim ();
	show_stat ();
	return 0;
}
