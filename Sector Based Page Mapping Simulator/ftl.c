//----------------------------------------------------------
//
// Lab #5 : Sector-based Page Mapping FTL Simulator
//    - Embedded System Design, ICE3028 (Fall, 2019)
//
// Oct. 17, 2019.
//
// Junho Lee, Somm Kim
// Dongkun Shin (dongkun@skku.edu)
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
	u32 p_age;
	char v;
}ptable;
ptable *ppn;
u32 *ptr;
int *freeblocks;

#define MAX		0xFFFFFFFF


static long now() {
	return s.ftl_write + s.gc_write;
}

void garbage_collection(u32 bank);

void lftl_read(u32 lpn, u32 *read_buffer)
{	
	int bank = lpn%N_BANKS;
	u32 addr = l2p[lpn].ppn;
	int blk = (addr - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (addr - bank*BLKS_PER_BANK*PAGES_PER_BLK - blk*PAGES_PER_BLK);
	u32 spare;
	nand_read(bank,blk,page,read_buffer,&spare);
	return;
}

void lftl_write(u32 lpn, u32 *write_buffer)
{	
	int bank = lpn%N_BANKS;
	if(freeblocks[bank] == 1) garbage_collection(bank);
	if(l2p[lpn].v != 'n')
	{
		u32 addr;
		addr = l2p[lpn].ppn;
		ppn[addr].v = 'i';
		ppn[addr].p_age = now();
	}
	if((ptr[bank]+1)%(BLKS_PER_BANK) == 0) freeblocks[bank]--;
	int blk = ptr[bank]/PAGES_PER_BLK;
	int page = ptr[bank]%PAGES_PER_BLK;
	l2p[lpn].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + blk*PAGES_PER_BLK + page;
	l2p[lpn].v = 'w';
	nand_write(bank,blk,page,write_buffer,lpn);
	ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK+ptr[bank]].v = 'v';
	if(ptr[bank] == BLKS_PER_BANK*PAGES_PER_BLK-1)
	{
		ptr[bank] = 0;
		while(ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank]].v != 'f') ptr[bank]++;
	}
	else ptr[bank]++;
	return;
}

void ftl_open()
{
	freeblocks = (int*)malloc(sizeof(int)*N_BANKS);
	int k;
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
	ptr = (u32*)malloc(sizeof(u32)*N_BANKS);
	for(i=0;i<N_BANKS;i++) ptr[i] = 0;
	return;
}

void ftl_read(u32 lba, u32 num_sectors, u32 *read_buffer)
{	
	//printf("read %d!\n",num_sectors);
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
	}
	return;
}

void ftl_write(u32 lba, u32 num_sectors, u32 *write_buffer)
{
	//printf("write %d!\n",num_sectors);
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
		lftl_write(lpn,buffer);
		s.ftl_write += SECTORS_PER_PAGE;
		write_buffer_ptr += num_sectors_to_write;
		sect_offset = 0;
		remain_sectors -= num_sectors_to_write;
		lpn++;
	}
/***************************************
	Add

	s.ftl_write += SECTORS_PER_PAGE;
	
	for every nand_write call
	that you issue in this function
***************************************/
	return;
}

void garbage_collection(u32 bank)
{
	s.gc++;

#ifndef COST_BENEFIT
	int i,j,count,st = 0;
	int nfree = 0;
	int victim = 0;
	int freeblk = 0;
	u32* spare = (u32*)malloc(sizeof(u32));
	u32 r_buffer[SECTORS_PER_PAGE];
	memset(r_buffer, 0, DATA_SIZE);
	u32 s_point = bank*BLKS_PER_BANK*PAGES_PER_BLK;
	for(i=0;i<BLKS_PER_BANK;i++)
	{
		count = 0;
		nfree = 0;
		for(j=0;j<PAGES_PER_BLK;j++)
		{
			if(ppn[s_point+i*PAGES_PER_BLK+j].v == 'i') count++;
			if(ppn[s_point+i*PAGES_PER_BLK+j].v == 'f') nfree++;
		}
		if(count>st)
		{	
			victim = i;
			st = count;
		}
		if(nfree == BLKS_PER_BANK){
			freeblk = i;
		}
	}
	ptr[bank] = freeblk*PAGES_PER_BLK;
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		if(ppn[s_point+victim*PAGES_PER_BLK+j].v == 'v')
		{
			nand_read(bank,victim,j,r_buffer,spare);
			nand_write(bank,freeblk,ptr[bank]%PAGES_PER_BLK,r_buffer,*spare);
			ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank]].v = 'v';
			l2p[*spare].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + freeblk*PAGES_PER_BLK + (ptr[bank]%PAGES_PER_BLK);
			ptr[bank]++;
			s.gc_write += SECTORS_PER_PAGE;
		}
		ppn[s_point+victim*PAGES_PER_BLK+j].v = 'f';
		ppn[s_point+victim*PAGES_PER_BLK+j].p_age = 0;
	}
	nand_erase(bank,victim);
	freeblocks[bank]++;
	free(spare);

#else
	int i,j,count = 0;
	float st = 0;
	float cost_b = 0;
	int nfree = 0;
	int victim = 0;
	int freeblk = 0;
	int age = 0;
	u32* spare = (u32*)malloc(sizeof(u32));
	u32 r_buffer[SECTORS_PER_PAGE];
	memset(r_buffer, 0, DATA_SIZE);
	u32 s_point = bank*BLKS_PER_BANK*PAGES_PER_BLK;
	for(i=0;i<BLKS_PER_BANK;i++)
	{
		count = 0;
		nfree = 0;
		age = 0;
		for(j=0;j<PAGES_PER_BLK;j++)
		{
			if((ppn[s_point+i*PAGES_PER_BLK+j].v == 'v') && (ppn[s_point+i*PAGES_PER_BLK+j].p_age > age))
			age = ppn[s_point+i*PAGES_PER_BLK+j].p_age;
			if(ppn[s_point+i*PAGES_PER_BLK+j].v == 'i') count++;
			if(ppn[s_point+i*PAGES_PER_BLK+j].v == 'f') nfree++;
		}
		if(count == PAGES_PER_BLK)
		{
			victim = i;
			st = count * now();
		}
		else cost_b = (count*(now()-age))/(PAGES_PER_BLK - count);	
		if(cost_b>st)
		{	
			victim = i;
			st = cost_b;
		}
		if(nfree == BLKS_PER_BANK){
			freeblk = i;
		}
	}
	ptr[bank] = freeblk*PAGES_PER_BLK;
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		if(ppn[s_point+victim*PAGES_PER_BLK+j].v == 'v')
		{
			nand_read(bank,victim,j,r_buffer,spare);
			nand_write(bank,freeblk,ptr[bank]%PAGES_PER_BLK,r_buffer,*spare);
			ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank]].v = 'v';
			l2p[*spare].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + freeblk*PAGES_PER_BLK + (ptr[bank]%PAGES_PER_BLK);
			ptr[bank]++;
			s.gc_write += SECTORS_PER_PAGE;
		}
		ppn[s_point+victim*PAGES_PER_BLK+j].v = 'f';

		ppn[s_point+victim*PAGES_PER_BLK+j].p_age = 0;

	}
	nand_erase(bank,victim);
	freeblocks[bank]++;
	free(spare);
#endif


	return;
}
