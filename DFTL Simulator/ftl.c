//----------------------------------------------------------
//
// Project #1 : DFTL Simulator
//    - Embedded System Design, ICE3028 (Fall, 2019)
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
#include "nand.h"

#define MAX			0xFFFFFFFF
#define MAX_LPN			N_BANKS*BLKS_PER_BANK*PAGES_PER_BLK
void garbage_collection(u32 bank);

void map_read(u32 bank, u32 map_page, u32 cache_slot);
void map_write(u32 bank, u32 map_page, u32 cache_slot);
void map_garbage_collection(u32 bank);

typedef struct l2p_table{
	char dirty; //D for dirty C for clean N for Not used yet
	u32 *ppn;
	u32 *lpn;
	u32 access_time;
}cache;

typedef struct caches{
	cache* l2p_cache;
}l2p;

l2p *CMT;

typedef struct memory_map{
	char used; // U for used N for Not used yet
	u32 mapped_page;
}map_table;

typedef struct g2m{
	map_table* gtd_cache;
}table;

table *GTD;

typedef struct ppn_table{
	char type;  // U for user M for map
	char vaild;   //I for invaild, V for Vaild, F for free
}ppn;

ppn *ppn_inf; 

u32 *free_page_ptr_user;
u32 *free_map_page;
u32 *free_user_page;
u32 *free_page_ptr_map;

static long now() {
	return s.ftl_write + s.gc_write;
}
void inc_user_page(u32 bank)
{
	free_page_ptr_user[bank]++;
	if(free_page_ptr_user[bank]%PAGES_PER_BLK == 0 )
	{
		if(free_page_ptr_user[bank] == BLKS_PER_BANK*PAGES_PER_BLK) free_page_ptr_user[bank] = 0;
		while(ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_user[bank]].vaild != 'F'||free_page_ptr_user[bank] == free_page_ptr_map[bank])
		{	
			free_page_ptr_user[bank] += PAGES_PER_BLK;
			if(free_page_ptr_user[bank] >= BLKS_PER_BANK*PAGES_PER_BLK) free_page_ptr_user[bank] = 0;
		}
	}
}
void inc_map_page(u32 bank)
{
	free_page_ptr_map[bank]++;
	if(free_page_ptr_map[bank]%PAGES_PER_BLK == 0 )
	{
		if(free_page_ptr_map[bank] == BLKS_PER_BANK*PAGES_PER_BLK) free_page_ptr_map[bank] = 0;
		while(ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank]].vaild != 'F'||free_page_ptr_user[bank] == free_page_ptr_map[bank])
		{	
			free_page_ptr_map[bank] += PAGES_PER_BLK;
			if(free_page_ptr_map[bank] >= BLKS_PER_BANK*PAGES_PER_BLK) free_page_ptr_map[bank] = 0;
		}
	}
}
void ftl_open()
{
	int i,j,k;
	CMT = (l2p*)malloc(sizeof(l2p)*N_BANKS);
	for(i=0;i<N_BANKS;i++) CMT[i].l2p_cache = (cache*)malloc(sizeof(cache)* N_CACHED_MAP_PAGE_PB);
	for(i=0;i<N_BANKS;i++)
	{
		for(j=0;j<N_CACHED_MAP_PAGE_PB;j++)
		{
			CMT[i].l2p_cache[j].ppn = (u32*)malloc(sizeof(u32)*N_MAP_ENTRIES_PER_PAGE);
			CMT[i].l2p_cache[j].lpn = (u32*)malloc(sizeof(u32)*N_MAP_ENTRIES_PER_PAGE);
			for(k=0;k<8;k++) CMT[i].l2p_cache[j].ppn[k] = MAX;
			CMT[i].l2p_cache[j].dirty = 'N';
			CMT[i].l2p_cache[j].access_time = 0;
		}
	}
	GTD = (table*)malloc(sizeof(table)*N_BANKS);
	for(i=0;i<N_BANKS;i++) GTD[i].gtd_cache = (map_table*)malloc(sizeof(map_table)* N_MAP_PAGES_PB);
	for(i=0;i<N_BANKS;i++)
	{
		for(j=0;j<N_MAP_PAGES_PB;j++)
		{
			for(k=0;k<8;k++) GTD[i].gtd_cache[j].used = 'N';
			GTD[i].gtd_cache[j].mapped_page = MAX;
		}
	}
	ppn_inf = (ppn*)malloc(sizeof(ppn)*N_BANKS*BLKS_PER_BANK*PAGES_PER_BLK);
	for(i=0;i<N_BANKS*BLKS_PER_BANK*PAGES_PER_BLK;i++)
	{
		ppn_inf[i].vaild = 'F';
		ppn_inf[i].type = 'N';
	}
	free_map_page = (u32*)malloc(sizeof(u32)*N_BANKS);
	free_user_page = (u32*)malloc(sizeof(u32)*N_BANKS);
	free_page_ptr_user = (u32*)malloc(sizeof(u32)*N_BANKS);
	free_page_ptr_map = (u32*)malloc(sizeof(u32)*N_BANKS);
	for(i=0;i<N_BANKS;i++)
	{
		free_map_page[i] = (N_MAP_BLOCKS_PB + N_OP_MAP_BLOCKS_PB) * PAGES_PER_BLK;
		free_user_page[i] = (N_USER_BLOCKS_PB+N_OP_USER_BLOCKS_PB) * PAGES_PER_BLK;
		free_page_ptr_user[i] = 0;
		free_page_ptr_map[i] = PAGES_PER_BLK;
		ppn_inf[i*BLKS_PER_BANK*PAGES_PER_BLK+free_page_ptr_user[i]].type = 'U';
		ppn_inf[i*BLKS_PER_BANK*PAGES_PER_BLK+free_page_ptr_user[i]].vaild = 'V';
		ppn_inf[i*BLKS_PER_BANK*PAGES_PER_BLK+free_page_ptr_map[i]].type = 'W';
		ppn_inf[i*BLKS_PER_BANK*PAGES_PER_BLK+free_page_ptr_map[i]].vaild = 'V';
	}
	nand_init(N_BANKS,BLKS_PER_BANK,PAGES_PER_BLK);
	return;
}

int CMT_search(int* cmt_index, int* ppn_index, u32 lpn , u32 bank)
{
	int i,j;
	for(i=0;i<N_CACHED_MAP_PAGE_PB;i++)
	{
		for(j=0;j<N_MAP_ENTRIES_PER_PAGE;j++)
		{
			if(CMT[bank].l2p_cache[i].lpn[j] == lpn)
			{
				*cmt_index = i;
				*ppn_index = j;
				s.cache_hit++;
				return 0;
			}
		}
	}
	s.cache_miss++;
	return 1;
}

void CMT_miss(int* cmt_index, int* ppn_index, u32 lpn , u32 bank)
{
	u32 i,idx,j = 0;
	u32 ppn = 0;
	u32 t = MAX;
	int victim = 0;
	idx = lpn/(8*N_BANKS);
	for(i=0;i<N_CACHED_MAP_PAGE_PB;i++)
	{
		if(CMT[bank].l2p_cache[i].access_time < t)
		{
			t = CMT[bank].l2p_cache[i].access_time;
			victim = i;
		}
	}
	u32 index = CMT[bank].l2p_cache[victim].lpn[0] / (8*N_BANKS);
	if(CMT[bank].l2p_cache[victim].dirty == 'D')
	{
		while(free_map_page[bank] == PAGES_PER_BLK*N_MAP_GC_THRESHOLD) map_garbage_collection(bank);
		if(GTD[bank].gtd_cache[index].used != 'N') ppn_inf[GTD[bank].gtd_cache[index].mapped_page].vaild = 'I';
		else GTD[bank].gtd_cache[index].used = 'U';
		map_write(bank,bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank],victim);
		GTD[bank].gtd_cache[index].mapped_page = bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank];
		ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank]].vaild = 'V';
		ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank]].type = 'M';
		free_map_page[bank]--;
		inc_map_page(bank);
	}
	if(GTD[bank].gtd_cache[idx].used == 'N') 
	{
		for(i=0;i<N_MAP_ENTRIES_PER_PAGE;i++)
		{
			if(i * N_BANKS + idx*N_BANKS*8 == lpn)
			{	
				CMT[bank].l2p_cache[victim].ppn[i] = ppn;
			}
			CMT[bank].l2p_cache[victim].lpn[i] = i * N_BANKS + idx*N_BANKS*8 + bank;
			CMT[bank].l2p_cache[victim].ppn[i] = MAX;
		}
		CMT[bank].l2p_cache[victim].access_time = now();
		CMT[bank].l2p_cache[victim].dirty = 'D';
	}
	else
	{
		ppn = GTD[bank].gtd_cache[idx].mapped_page;
		map_read(bank,ppn,victim);
	}
	for(i=0;i<N_CACHED_MAP_PAGE_PB;i++)
	{
		for(j=0;j<N_MAP_ENTRIES_PER_PAGE;j++)
		{
			if(CMT[bank].l2p_cache[i].lpn[j] == lpn)
			{
				*cmt_index = i;
				*ppn_index = j;
				break;
			}
		}
	}
	return ;
}

void ftl_read_page(u32 lpn, u32 *read_buffer)
{	
	int cmt_index = 0;
	int ppn_index = 0;
	int bank = lpn%N_BANKS;
	if(CMT_search(&cmt_index,&ppn_index,lpn,bank)) CMT_miss(&cmt_index,&ppn_index,lpn,bank);
	if(CMT[bank].l2p_cache[cmt_index].ppn[ppn_index] == MAX) return;
	int blk = (CMT[bank].l2p_cache[cmt_index].ppn[ppn_index] - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (CMT[bank].l2p_cache[cmt_index].ppn[ppn_index] - bank*BLKS_PER_BANK*PAGES_PER_BLK)%PAGES_PER_BLK;
	u32 spare;
	nand_read(bank,blk,page,read_buffer,&spare);
	return;
}

void ftl_write_page(u32 lpn, u32 *write_buffer)
{	
	int bank = lpn%N_BANKS;
	while(free_user_page[bank] == PAGES_PER_BLK*N_GC_THRESHOLD) garbage_collection(bank);
	int cmt_index,ppn_index;
	if(CMT_search(&cmt_index,&ppn_index,lpn,bank)) CMT_miss(&cmt_index,&ppn_index,lpn,bank);
	u32 ppn = bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_user[bank];
	if(CMT[bank].l2p_cache[cmt_index].ppn[ppn_index] != MAX) ppn_inf[CMT[bank].l2p_cache[cmt_index].ppn[ppn_index]].vaild = 'I';
	CMT[bank].l2p_cache[cmt_index].ppn[ppn_index] = ppn;
	CMT[bank].l2p_cache[cmt_index].dirty = 'D';
	CMT[bank].l2p_cache[cmt_index].access_time = now();
	int blk = (ppn - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (ppn - bank*BLKS_PER_BANK*PAGES_PER_BLK - blk*PAGES_PER_BLK);
	nand_write(bank,blk,page,write_buffer,lpn);
	ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_user[bank]].vaild = 'V';
	ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_user[bank]].type = 'U';
	free_user_page[bank]--;
	inc_user_page(bank);
	return;
}

void ftl_read(u32 lba, u32 num_sectors, u32 *read_buffer)
{
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
		ftl_read_page(lpn,buffer);
		for(i=0;i<num_sectors_to_read;i++)
		{
			read_buffer[read_buffer_ptr + i] = buffer[sect_offset + i];
		}
		read_buffer_ptr += num_sectors_to_read;
		sect_offset = 0;
		sectors_remain -=num_sectors_to_read;
		lpn++;
		if(lpn == MAX_LPN) lpn = 0;
	}
	return;
}

void ftl_write(u32 lba, u32 num_sectors, u32 *write_buffer)
{
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
		if(sect_offset != 0)
		{
			ftl_read_page(lpn,buffer);
		}
		for(i=0;i<num_sectors_to_write;i++)
		{
			buffer[sect_offset + i] = write_buffer[write_buffer_ptr + i];
		}
		ftl_write_page(lpn,buffer);
		s.ftl_write += SECTORS_PER_PAGE;
		write_buffer_ptr += num_sectors_to_write;
		sect_offset = 0;
		remain_sectors -= num_sectors_to_write;
		lpn++;
		if(lpn == MAX_LPN) lpn = 0;
	}	
}

void garbage_collection(u32 bank)
{
	s.gc++;
	int i,j,invaild_count,vaild_count,store_value = 0;
	int victim = 0;
	int free_count = 0;
	u32 spare;
	u32 r_buffer[SECTORS_PER_PAGE];
	memset(r_buffer, 0, DATA_SIZE);
	u32 s_point = bank*BLKS_PER_BANK*PAGES_PER_BLK;
	for(i=BLKS_PER_BANK-1;i>=0;i--)
	{
		invaild_count = 0;
		free_count = 0;
		for(j=0;j<PAGES_PER_BLK;j++)
		{
			if(ppn_inf[s_point+i*PAGES_PER_BLK].type == 'U' && ppn_inf[s_point+i*PAGES_PER_BLK+j].vaild == 'I') invaild_count++;
			if(ppn_inf[s_point+i*PAGES_PER_BLK+j].vaild == 'F') free_count++;
		}
		if(invaild_count>store_value)
		{	
			victim = i;
			store_value = invaild_count;
		}
		if(free_count == PAGES_PER_BLK && free_page_ptr_map[bank] != i*PAGES_PER_BLK) free_page_ptr_user[bank] = i*PAGES_PER_BLK;
	}
	if(store_value == 0) return;
	vaild_count = 0;
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		if(ppn_inf[s_point+victim*PAGES_PER_BLK+j].vaild == 'V')
		{
			vaild_count++;
			nand_read(bank,victim,j,r_buffer,&spare);
			free_user_page[bank]++;
			ftl_write_page(spare,r_buffer);
			s.gc_write += SECTORS_PER_PAGE;
		}
	}
	nand_erase(bank,victim);
	free_user_page[bank] += PAGES_PER_BLK - vaild_count;
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		ppn_inf[s_point+victim*PAGES_PER_BLK+j].vaild = 'F';
		ppn_inf[s_point+victim*PAGES_PER_BLK+j].type = 'N';
	}
}

void map_read(u32 bank, u32 map_page, u32 cache_slot)
{
	u32 *read_buffer = (u32 *)calloc(SECTORS_PER_PAGE, sizeof(u32));
	u32 spare; 
	int i;
	int blk = (map_page - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (map_page - bank*BLKS_PER_BANK*PAGES_PER_BLK - blk*PAGES_PER_BLK);
	nand_read(bank,blk,page,read_buffer,&spare);
	for(i=0;i<N_MAP_ENTRIES_PER_PAGE;i++)
	{
		CMT[bank].l2p_cache[cache_slot].lpn[i] = spare;
		spare += N_BANKS;
		CMT[bank].l2p_cache[cache_slot].ppn[i] = read_buffer[i];
	}
	CMT[bank].l2p_cache[cache_slot].access_time = now();
	CMT[bank].l2p_cache[cache_slot].dirty = 'C';	
}

void map_write(u32 bank, u32 map_page, u32 cache_slot)
{
	u32 *write_buffer = (u32 *)calloc(SECTORS_PER_PAGE, sizeof(u32));
	u32 spare = CMT[bank].l2p_cache[cache_slot].lpn[0];
	int i;
	int blk = (map_page - bank*BLKS_PER_BANK*PAGES_PER_BLK)/PAGES_PER_BLK;
	int page = (map_page - bank*BLKS_PER_BANK*PAGES_PER_BLK - blk*PAGES_PER_BLK);
	for(i=0;i<N_MAP_ENTRIES_PER_PAGE;i++)	write_buffer[i] = CMT[bank].l2p_cache[cache_slot].ppn[i];
	nand_write(bank,blk,page,write_buffer,spare);
	s.map_write += SECTORS_PER_PAGE;	
}

void map_garbage_collection(u32 bank)
{
	s.map_gc++;
	int i,j,invaild_count,store_value = 0;
	int vaild_count,victim = 0;
	u32 spare;
	int free_count = 0;
	u32 r_buffer[SECTORS_PER_PAGE];
	memset(r_buffer, 0, DATA_SIZE);
	u32 s_point = bank*BLKS_PER_BANK*PAGES_PER_BLK;
	for(i=0;i<BLKS_PER_BANK;i++)
	{
		invaild_count = 0;
		free_count = 0;
		for(j=0;j<PAGES_PER_BLK;j++)
		{
			if(ppn_inf[s_point+i*PAGES_PER_BLK].type == 'M' && ppn_inf[s_point+i*PAGES_PER_BLK+j].vaild == 'I') invaild_count++;
			if(ppn_inf[s_point+i*PAGES_PER_BLK+j].vaild == 'F') free_count++;
		}
		if(invaild_count>store_value)
		{	
			victim = i;
			store_value = invaild_count;
		}
		if(free_count == PAGES_PER_BLK && free_page_ptr_user[bank] != i*PAGES_PER_BLK) free_page_ptr_map[bank] = i*PAGES_PER_BLK;
	}
	vaild_count = 0;
	if(store_value == 0) return;
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		if(ppn_inf[s_point+victim*PAGES_PER_BLK+j].vaild == 'V')
		{
			vaild_count++;
			nand_read(bank,victim,j,r_buffer,&spare);
			nand_write(bank,free_page_ptr_map[bank]/PAGES_PER_BLK,free_page_ptr_map[bank]%PAGES_PER_BLK,r_buffer,spare);
			GTD[bank].gtd_cache[spare/(8*N_BANKS)].mapped_page = bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank];
			ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank]].vaild = 'V';
			ppn_inf[bank*BLKS_PER_BANK*PAGES_PER_BLK + free_page_ptr_map[bank]].type = 'M';
			inc_map_page(bank);
			s.map_gc_write += SECTORS_PER_PAGE;
		}
	}
	for(j=0;j<PAGES_PER_BLK;j++)
	{
		ppn_inf[s_point+victim*PAGES_PER_BLK+j].vaild = 'F';
		ppn_inf[s_point+victim*PAGES_PER_BLK+j].type = 'N';
	}
	nand_erase(bank,victim);
	free_map_page[bank] += PAGES_PER_BLK - vaild_count;	
}
