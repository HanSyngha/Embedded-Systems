// Copyright 2011 INDILINX Co., Ltd.
//
// This file is part of Jasmine.
//
// Jasmine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Jasmine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Jasmine. See the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//
// GreedyFTL header file
//
// Author; Sang-Phil Lim (SKKU VLDB Lab.)
//

#ifndef FTL_H
#define FTL_H


/////////////////
// DRAM buffers
/////////////////

#define NUM_RW_BUFFERS		((DRAM_SIZE - DRAM_BYTES_OTHER) / BYTES_PER_PAGE - 1)
#define NUM_RD_BUFFERS		(((NUM_RW_BUFFERS / 8) + NUM_BANKS - 1) / NUM_BANKS * NUM_BANKS)
#define NUM_WR_BUFFERS		(NUM_RW_BUFFERS - NUM_RD_BUFFERS)
#define NUM_COPY_BUFFERS	NUM_BANKS_MAX
#define NUM_FTL_BUFFERS		NUM_BANKS
#define NUM_HIL_BUFFERS		1
#define NUM_TEMP_BUFFERS	1

#define DRAM_BYTES_OTHER	((NUM_COPY_BUFFERS + NUM_FTL_BUFFERS + NUM_HIL_BUFFERS + NUM_TEMP_BUFFERS) * BYTES_PER_PAGE \
+ BAD_BLK_BMP_BYTES + VCOUNT_BYTES + CMT_BYTES + GTD_BYTES + VCOUNT_MAP_BYTES + MAP_BUF_BYTES)

#define WR_BUF_PTR(BUF_ID)	(WR_BUF_ADDR + ((UINT32)(BUF_ID)) * BYTES_PER_PAGE)
#define WR_BUF_ID(BUF_PTR)	((((UINT32)BUF_PTR) - WR_BUF_ADDR) / BYTES_PER_PAGE)
#define RD_BUF_PTR(BUF_ID)	(RD_BUF_ADDR + ((UINT32)(BUF_ID)) * BYTES_PER_PAGE)
#define RD_BUF_ID(BUF_PTR)	((((UINT32)BUF_PTR) - RD_BUF_ADDR) / BYTES_PER_PAGE)

#define _COPY_BUF(RBANK)	(COPY_BUF_ADDR + (RBANK) * BYTES_PER_PAGE)
#define COPY_BUF(BANK)		_COPY_BUF(REAL_BANK(BANK))
#define FTL_BUF(BANK)       (FTL_BUF_ADDR + ((BANK) * BYTES_PER_PAGE))
#define MAP_BUF(BANK)       (MAP_BUF_ADDR + ((BANK) * BYTES_PER_PAGE))

///////////////////////////////
// DRAM segmentation
///////////////////////////////

#define RD_BUF_ADDR			DRAM_BASE										// base address of SATA read buffers
#define RD_BUF_BYTES		(NUM_RD_BUFFERS * BYTES_PER_PAGE)

#define WR_BUF_ADDR			(RD_BUF_ADDR + RD_BUF_BYTES)					// base address of SATA write buffers
#define WR_BUF_BYTES		(NUM_WR_BUFFERS * BYTES_PER_PAGE)

#define COPY_BUF_ADDR		(WR_BUF_ADDR + WR_BUF_BYTES)					// base address of flash copy buffers
#define COPY_BUF_BYTES		(NUM_COPY_BUFFERS * BYTES_PER_PAGE)

#define FTL_BUF_ADDR		(COPY_BUF_ADDR + COPY_BUF_BYTES)				// a buffer dedicated to FTL internal purpose
#define FTL_BUF_BYTES		(NUM_FTL_BUFFERS * BYTES_PER_PAGE)

#define HIL_BUF_ADDR		(FTL_BUF_ADDR + FTL_BUF_BYTES)					// a buffer dedicated to HIL internal purpose
#define HIL_BUF_BYTES		(NUM_HIL_BUFFERS * BYTES_PER_PAGE)

#define TEMP_BUF_ADDR		(HIL_BUF_ADDR + HIL_BUF_BYTES)					// general purpose buffer
#define TEMP_BUF_BYTES		(NUM_TEMP_BUFFERS * BYTES_PER_PAGE)

#define BAD_BLK_BMP_ADDR	(TEMP_BUF_ADDR + TEMP_BUF_BYTES)				// bitmap of initial bad blocks
#define BAD_BLK_BMP_BYTES	(((NUM_VBLKS / 8) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

#define CMT_ADDR		    (BAD_BLK_BMP_ADDR + BAD_BLK_BMP_BYTES)
#define CMT_BYTES		    (NUM_BANKS * (N_CACHED_MAP_PAGE_PB * CMT_ROW_BYTES))    // LPN + Entries + Dirty + Time

#define GTD_ADDR		    (CMT_ADDR + CMT_BYTES)
#define GTD_BYTES		    (NUM_BANKS * GTD_ROW_BYTES)

#define VCOUNT_ADDR			(GTD_ADDR + GTD_BYTES)
#define VCOUNT_BYTES		((NUM_BANKS * VBLKS_PER_BANK * sizeof(UINT16) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR * BYTES_PER_SECTOR)

#define VCOUNT_MAP_ADDR		(VCOUNT_ADDR + VCOUNT_BYTES)
#define VCOUNT_MAP_BYTES	((NUM_BANKS * VBLKS_PER_BANK * sizeof(UINT16) + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR * BYTES_PER_SECTOR)

#define MAP_BUF_ADDR        (VCOUNT_MAP_ADDR + VCOUNT_MAP_BYTES)
#define MAP_BUF_BYTES       (NUM_BANKS * BYTES_PER_PAGE)

#define BLKS_PER_BANK		VBLKS_PER_BANK

//-------------------
// Project Test Code
//-------------------
#define MAP_ENTRY_SIZE          sizeof(UINT32)
#define N_MAP_ENTRIES_PER_PAGE  ((SECTORS_PER_PAGE * BYTES_PER_SECTOR) / MAP_ENTRY_SIZE)
#define N_MAP_PAGES_PB          ((NUM_LPAGES + N_MAP_ENTRIES_PER_PAGE) / N_MAP_ENTRIES_PER_PAGE)
#define TMP_MAP_BLOCKS_PB       ((N_MAP_PAGES_PB + PAGES_PER_BLK) / PAGES_PER_BLK)

#define MAP_OP_RATIO            (7)
#define REMAINING_BLOCKS        (VBLKS_PER_BANK - 4)    // except block #0, #1, #2, #3
#define N_MAP_OP_BLOCKS_PB      (REMAINING_BLOCKS * MAP_OP_RATIO / 100)
#define N_MAP_BLOCKS_PB         (TMP_MAP_BLOCKS_PB + N_MAP_OP_BLOCKS_PB)

#define CMT_RATIO               (25)
#define N_CACHED_MAP_PAGE_PB    (N_MAP_PAGES_PB * CMT_RATIO / 100)

#define GTD_ROW_BYTES           ((((PAGES_PER_BANK / N_MAP_ENTRIES_PER_PAGE) * sizeof(UINT32)) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)
#define CMT_ROW_BYTES           ((((N_MAP_ENTRIES_PER_PAGE * MAP_ENTRY_SIZE) + sizeof(UINT32) * 3) + DRAM_ECC_UNIT - 1) / DRAM_ECC_UNIT * DRAM_ECC_UNIT)

///////////////////////////////
// FTL public functions
///////////////////////////////

void ftl_open(void);
void ftl_read(UINT32 const lba, UINT32 const num_sectors);
void ftl_write(UINT32 const lba, UINT32 const num_sectors);
void ftl_test_write(UINT32 const lba, UINT32 const num_sectors);
void ftl_flush(void);
void ftl_isr(void);

#endif //FTL_H