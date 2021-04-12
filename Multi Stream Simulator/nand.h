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


typedef unsigned int 		u32;

#define DATA_SIZE			(sizeof(u32) * 8)
#define SPARE_SIZE			(sizeof(u32))

// function prototypes
int nand_init(int nbanks, int nblks, int npages);
int nand_read(int bank, int blk, int page, u32 *data, u32 *spare);
int nand_write(int bank, int blk, int page, u32 *data, u32 spare);
int nand_erase(int bank, int blk);
int nand_blkdump(int bank, int blk);

