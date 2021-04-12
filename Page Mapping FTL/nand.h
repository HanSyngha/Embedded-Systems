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


typedef unsigned int 		u32;

#define DATA_SIZE			(sizeof(u32) * 8)
#define SPARE_SIZE			(sizeof(u32))

// function prototypes
int nand_init(int nbanks, int nblks, int npages);
int nand_read(int bank, int blk, int page, u32 *data, u32 *spare);
int nand_write(int bank, int blk, int page, u32 *data, u32 spare);
int nand_erase(int bank, int blk);
int nand_blkdump(int bank, int blk);

