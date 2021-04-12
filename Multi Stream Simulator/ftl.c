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
#include "nand.h"
typedef struct l2ptable
{
	u32 ppn;
	char v;
}table;
table* l2p;
typedef struct ppntable
{
	char v;
	int s_id;
}ptable;
ptable *ppn;
u32 **ptr;
int *freeblocks;

#define MAX		0xFFFFFFFF

static void garbage_collection(u32 bank, u32 streamid);

void inc_ptr(int bank, int s_id)
{
	u32 starting_addr = bank*BLKS_PER_BANK*PAGES_PER_BLK;
	if(ppn[starting_addr + ptr[bank][s_id]].v == 'f' || ppn[starting_addr + ptr[bank][s_id]].v == 'u') return;
	ptr[bank][s_id]++;
	while(1)
	{
		if(ptr[bank][s_id] >= BLKS_PER_BANK*PAGES_PER_BLK) ptr[bank][s_id] = 0;
		if(ppn[starting_addr + ptr[bank][s_id]].v == 'f') break;
		else if(ppn[starting_addr + ptr[bank][s_id]].v != 'f') ptr[bank][s_id] += PAGES_PER_BLK;
	}
	ppn[starting_addr + ptr[bank][s_id]].v = 'u';
	ppn[starting_addr + ptr[bank][s_id]].s_id = s_id;
}
void lftl_read(u32 lpn, u32 *read_buffer)
{	
	if(ppn[l2p[lpn].ppn].v != 'v') return;
	int bank = lpn%N_BANKS;
	u32 addr = l2p[lpn].ppn;
	int blk = (addr - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (addr - bank*BLKS_PER_BANK*PAGES_PER_BLK)%PAGES_PER_BLK;
	u32 spare;
	nand_read(bank,blk,page,read_buffer,&spare);
	return;
}

void lftl_write(u32 lpn, u32 *write_buffer, int streamid)
{	
	int gc_num = 0;
	int bank = lpn%N_BANKS;
	int s_id = streamid/3;
	int stream = streamid;
	inc_ptr(bank,s_id);
	if(ptr[bank][s_id]%PAGES_PER_BLK == 0) freeblocks[bank]--;
	while(freeblocks[bank] == 1)
	{	
		gc_num = 1;
		garbage_collection(bank,stream);
		if(stream == 6) stream = 0;
		else stream += 3;
	}
	if(gc_num) inc_ptr(bank,s_id);
	if(ppn[l2p[lpn].ppn].v == 'v') ppn[l2p[lpn].ppn].v = 'i';
	int blk = ptr[bank][s_id]/PAGES_PER_BLK;
	int page = ptr[bank][s_id]%PAGES_PER_BLK;
	l2p[lpn].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + blk*PAGES_PER_BLK + page;
	l2p[lpn].v = 'w';
	nand_write(bank,blk,page,write_buffer,lpn);
	ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK+ptr[bank][s_id]].v = 'v';
	return;
}

void ftl_open(void)
{
	freeblocks = (int*)malloc(sizeof(int)*N_BANKS);
	int j,k;
	for(k=0;k<N_BANKS;k++) freeblocks[k] = BLKS_PER_BANK;
	u32 freepages = N_BANKS*BLKS_PER_BANK*PAGES_PER_BLK;
	nand_init(N_BANKS,BLKS_PER_BANK,PAGES_PER_BLK);
	l2p = (table*)malloc(sizeof(table)*freepages);
	ppn = (ptable*)malloc(sizeof(ptable)*freepages);
	u32 i;
	for(i=0;i<freepages;i++)
	{
		l2p[i].v = 'n';
		ppn[i].v = 'f';
	}
	ptr = (u32**)malloc(sizeof(u32*)*N_BANKS);
	for(i=0;i<N_BANKS;i++) ptr[i] = (u32*)malloc(sizeof(u32)*3);
	for(i=0;i<N_BANKS;i++) for(j=0;j<3;j++) ptr[i][j] = j*PAGES_PER_BLK;
	for(i=0;i<N_BANKS;i++) for(j=0;j<3;j++)
	{
		ppn[i*BLKS_PER_BANK*PAGES_PER_BLK + ptr[i][j]].s_id = j;
	}
	return;
}

void ftl_read(u32 lba, u32 num_sectors, u32 *read_buffer)
{
	//printf("read!\n");
	u32 *buffer = (u32 *)calloc(SECTORS_PER_PAGE, sizeof(u32));
	int i;
	u32 read_buffer_ptr = 0;
	u32 num_sectors_to_read;
	u32 lpn = lba / SECTORS_PER_PAGE;
	u32 sect_offset = lba % SECTORS_PER_PAGE;
	u32 sectors_remain = num_sectors;
	while(sectors_remain != 0)
	{
		if(sect_offset + sectors_remain < SECTORS_PER_PAGE) num_sectors_to_read = sectors_remain;
		else num_sectors_to_read = SECTORS_PER_PAGE - sect_offset;
		lftl_read(lpn,buffer);
		for(i=0;i<num_sectors_to_read;i++)
		{
			read_buffer[read_buffer_ptr + i] = buffer[sect_offset + i];
		}
		read_buffer_ptr += num_sectors_to_read;
		sect_offset = 0;
		sectors_remain -=num_sectors_to_read;
		lpn++;
		if(lpn == N_BANKS*BLKS_PER_BANK*PAGES_PER_BLK) lpn = 0;
	}
	return;
}

void ftl_write(u32 streamid, u32 lba, u32 num_sectors, u32 *write_buffer)
{
	//printf("wirte!\n");
	u32 num_sectors_to_write;
	u32 *buffer = (u32 *)calloc(SECTORS_PER_PAGE, sizeof(u32));
	int i;
	u32 write_buffer_ptr = 0;
	u32 lpn = lba / SECTORS_PER_PAGE;
	u32 sect_offset = lba % SECTORS_PER_PAGE;
	u32 remain_sectors = num_sectors;
	while(remain_sectors != 0)
	{
		for(i=0;i<SECTORS_PER_PAGE;i++) buffer[i] = 0;
		if(sect_offset + remain_sectors >= SECTORS_PER_PAGE) num_sectors_to_write = SECTORS_PER_PAGE - sect_offset;
		else num_sectors_to_write = remain_sectors;
		if(sect_offset != 0 && l2p[lpn].v != 'n')
		{
			lftl_read(lpn,buffer);
		}
		for(i=0;i<num_sectors_to_write;i++)
		{
			buffer[sect_offset + i] = write_buffer[write_buffer_ptr + i];
		}
		lftl_write(lpn,buffer,streamid);
		s.ftl_write += SECTORS_PER_PAGE;
		write_buffer_ptr += num_sectors_to_write;
		sect_offset = 0;
		remain_sectors -= num_sectors_to_write;
		lpn++;
		if(lpn == N_BANKS*BLKS_PER_BANK*PAGES_PER_BLK) lpn = 0;
	}
	return;
}

static void garbage_collection(u32 bank, u32 streamid)
{
	s.gc++;
	int i,j,count,st = 0;
	int s_id = streamid/3;
	int victim = 0;
	u32 spare;
	u32 r_buffer[SECTORS_PER_PAGE];
	memset(r_buffer, 0, DATA_SIZE);
	u32 s_point = bank*BLKS_PER_BANK*PAGES_PER_BLK;
	for(i=0;i<BLKS_PER_BANK;i++)
	{
		if(ppn[s_point+i*PAGES_PER_BLK].s_id != s_id) continue;
		count = 0;
		for(j=0;j<PAGES_PER_BLK;j++)
		{
			if(ppn[s_point+i*PAGES_PER_BLK+j].v == 'i') count++;
		}
		if(count>=st)
		{	
			victim = i;
			st = count;
		}
	}
	if(st == 0) return;
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		if(ppn[s_point+victim*PAGES_PER_BLK+j].v == 'v')
		{
			inc_ptr(bank,s_id);
			nand_read(bank,victim,j,r_buffer,&spare);
			nand_write(bank,ptr[bank][s_id]/PAGES_PER_BLK,ptr[bank][s_id]%PAGES_PER_BLK,r_buffer,spare);
			ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank][s_id]].v = 'v';
			ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank][s_id]].s_id = ppn[s_point+victim*PAGES_PER_BLK+j].s_id;
			l2p[spare].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank][s_id];
			s.gc_write += SECTORS_PER_PAGE;
		}
	}
	for(j=0;j<PAGES_PER_BLK;j++) ppn[s_point+victim*PAGES_PER_BLK+j].v = 'f';
	nand_erase(bank,victim);
	freeblocks[bank]++;
	//printf("GCEND\n");
	return;
}
