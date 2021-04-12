//----------------------------------------------------------
//
// Lab #4 : Page Mapping FTL Simulator
//    - Embedded System Design, ICE3028 (Fall, 2019)
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
u32 ltime = 1;


#ifdef COST_BENEFIT
/*static long now() {
	return s.host_write + s.gc_write;
}*/
#endif

void garbage_collection(u32 bank);

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

void ftl_read(u32 lpn, u32 *read_buffer)
{	
	int bank = lpn%N_BANKS;
	u32 addr = l2p[lpn].ppn;
	int blk = (addr - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (addr - bank*BLKS_PER_BANK*PAGES_PER_BLK - blk*PAGES_PER_BLK);
	u32 spare;
	nand_read(bank,blk,page,read_buffer,&spare);
	return;
}

void ftl_write(u32 lpn, u32 *write_buffer)
{	
	int bank = lpn%N_BANKS;
	if(freeblocks[bank] == 1) garbage_collection(bank);
	if(l2p[lpn].v != 'n')
	{
		u32 addr;
		addr = l2p[lpn].ppn;
		ppn[addr].v = 'i';
	}
	if((ptr[bank]+1)%(BLKS_PER_BANK) == 0) freeblocks[bank]--;
	int blk = ptr[bank]/PAGES_PER_BLK;
	int page = ptr[bank]%PAGES_PER_BLK;
	l2p[lpn].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + blk*PAGES_PER_BLK + page;
	l2p[lpn].v = 'w';
	nand_write(bank,blk,page,write_buffer,lpn);
	ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK+ptr[bank]].v = 'v';
	ppn[l2p[lpn].ppn].p_age = ltime;
	ltime++;
	if(ptr[bank] == BLKS_PER_BANK*PAGES_PER_BLK-1)
	{
		ptr[bank] = 0;
		while(ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank]].v != 'f') ptr[bank]++;
	}
	else ptr[bank]++;
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
			s.gc_write++;
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
			st = count * ltime;
		}
		else 	cost_b = (count*(ltime-age))/(PAGES_PER_BLK - count);	
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
			ppn[bank*BLKS_PER_BANK*PAGES_PER_BLK + ptr[bank]].p_age = ltime;
			ltime++;
			l2p[*spare].ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + freeblk*PAGES_PER_BLK + (ptr[bank]%PAGES_PER_BLK);
			ptr[bank]++;
			s.gc_write++;
		}
		ppn[s_point+victim*PAGES_PER_BLK+j].v = 'f';

		ppn[s_point+victim*PAGES_PER_BLK+j].p_age = 0;

	}
	nand_erase(bank,victim);
	freeblocks[bank]++;
	free(spare);
#endif

/***************************************
Add

s.gc_write++;

for every nand_write call (every valid page copy)
that you issue in this function
***************************************/

	return;
}
