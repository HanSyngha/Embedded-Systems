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


#include "jasmine.h"

UINT32 g_ftl_read_buf_id;
UINT32 g_ftl_write_buf_id;

// The Dummy FTL does not access NAND flash at all.
// You use Dummy FTL to measure SATA and DRAM speed.
// Since data are not written at all, benchmarks that do read-and-verify cannot be run over this FTL.

void ftl_open(void)
{
}

void ftl_read(UINT32 const lba, UINT32 const total_sectors)
{
	UINT32 num_sectors_to_read;

	UINT32 lpn				= lba / SECTORS_PER_PAGE;	// logical page address
	UINT32 sect_offset 		= lba % SECTORS_PER_PAGE;	// sector offset within the page
	UINT32 sectors_remain	= total_sectors;

	while (sectors_remain != 0)	// one page per iteration
	{
		if (sect_offset + sectors_remain < SECTORS_PER_PAGE)
		{
			num_sectors_to_read = sectors_remain;
		}
		else
		{
			num_sectors_to_read = SECTORS_PER_PAGE - sect_offset;
		}

		UINT32 next_read_buf_id = (g_ftl_read_buf_id + 1) % NUM_RD_BUFFERS;

		read_dram(lpn, sect_offset, num_sectors_to_read);

		while (next_read_buf_id == GETREG(SATA_RBUF_PTR));	// wait if the read buffer is full (slow host)

		SETREG(BM_STACK_RDSET, next_read_buf_id);	// change bm_read_limit
		SETREG(BM_STACK_RESET, 0x02);				// change bm_read_limit

		g_ftl_read_buf_id = next_read_buf_id;

		sect_offset = 0;
		sectors_remain -= num_sectors_to_read;
		lpn++;
	}
}

void ftl_write(UINT32 const lba, UINT32 const total_sectors)
{
	UINT32 num_sectors_to_write;

	UINT32 lpn				= lba / SECTORS_PER_PAGE;	// logical page address
	UINT32 sect_offset		= lba % SECTORS_PER_PAGE;
	UINT32 remain_sectors	= total_sectors;

	while (remain_sectors != 0)
	{
		if (sect_offset + remain_sectors >= SECTORS_PER_PAGE)
		{
			num_sectors_to_write = SECTORS_PER_PAGE - sect_offset;
		}
		else
		{
			num_sectors_to_write = remain_sectors;
		}

		while (g_ftl_write_buf_id == GETREG(SATA_WBUF_PTR));	// bm_write_limit should not outpace SATA_WBUF_PTR

		write_dram(lpn, sect_offset, num_sectors_to_write);

		g_ftl_write_buf_id = (g_ftl_write_buf_id + 1) % NUM_WR_BUFFERS;		// Circular buffer

		SETREG(BM_STACK_WRSET, g_ftl_write_buf_id);	// change bm_write_limit
		SETREG(BM_STACK_RESET, 0x01);				// change bm_write_limit

		sect_offset = 0;
		remain_sectors -= num_sectors_to_write;
	}
}

void read_dram(UINT32 const lpn, UINT32 const sect_offset, UINT32 const num_sectors_to_read)
{
	// Read from DRAM NEW & Write to SATA Read Buffer.
	UINT32 left_bytes = num_sectors_to_read * BYTES_PER_SECTOR;
	UINT32 offset_bytes = sect_offset * BYTES_PER_SECTOR;
	UINT32 start_byte = lpn * BYTES_PER_PAGE;

	mem_copy(RD_BUF_PTR(g_ftl_read_buf_id) + offset_bytes, DRAM_NEW + start_byte + offset_bytes, left_bytes);
}

void write_dram(UINT32 const lpn, UINT32 const sect_offset, UINT32 const num_sectors_to_write)
{
	// Read from SATA Write Buffer & Write to Empty Space.
	UINT32 left_bytes = num_sectors_to_write * BYTES_PER_SECTOR;
	UINT32 offset_bytes = sect_offset * BYTES_PER_SECTOR;
	UINT32 start_byte = lpn * BYTES_PER_PAGE;

	mem_copy(DRAM_NEW + start_byte + offset_bytes, WR_BUF_PTR(g_ftl_write_buf_id) + offset_bytes, left_bytes);
}

void ftl_flush(void)
{
}

void ftl_isr(void)
{
}

