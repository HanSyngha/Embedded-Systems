//----------------------------------------------------------
//
// Project #1 : NAND Simulator
// 	- Embedded Systems Design, ICE3028 (Fall, 2019)
//
// Sep. 19, 2019.
//
// TA: Junho Lee, Somm Kim
// Prof: Dongkun Shin
// Embedded Software Laboratory
// Sungkyunkwan University
// http://nyx.skku.ac.kr
//
//---------------------------------------------------------


typedef unsigned int 	u32;

#define BANK_SHIFT					1
#define BLOCK_SHIFT					3
#define PAGES_PER_BLOCK_SHIFT		3

#define N_BANKS						(1 << BANK_SHIFT)

#define SECTORS_PER_PAGE			8

#define PAGES_PER_BLK				(1 << PAGES_PER_BLOCK_SHIFT)
#define BLKS_PER_BANK				(1 << BLOCK_SHIFT)

#define DATA_SIZE					(sizeof(u32) * SECTORS_PER_PAGE)
#define SPARE_SIZE					(sizeof(u32))

// function prototypes
int nand_init(int nbanks, int nblks, int npages);
int nand_read(int bank, int blk, int page, u32 *data, u32 *spare);
int nand_write(int bank, int blk, int page, u32 *data, u32 spare);
int nand_erase(int bank, int blk);
int nand_blkdump(int bank, int blk);

