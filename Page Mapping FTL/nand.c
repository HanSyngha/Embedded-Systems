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
// http://nyx.ssku.ac.kr
//
//---------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "nand.h"
#define pgsize 74


int Banknum;
int PPB; //page per block
int BPB; //block per bank
int DPB; //byte per block

int nand_init(int nbanks, int nblks, int npages)
{
	// initialize the NAND flash memory 
	// "nblks": the total number of flash blocks per bank
	// "npages": the number of pages per block
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	int i,j,k,x;
	Banknum = nbanks;
	PPB = npages;
	BPB = nblks;
	DPB = (pgsize * PPB)+1;
	char* name = (char*)malloc(sizeof(char)*10);
	char* num = (char*)malloc(sizeof(char)*12);
	for(i = 0;i<nbanks;i++)
	{
		sprintf(num,"%d",i);
		strcpy(name,"bank_");
		strcat(name,num);
		int nand = open(name,O_CREAT|O_RDWR|O_TRUNC,0777);
		if(nand == -1)
		{
			if(write(1,"ERROR While Making Bank",50));
			return -1;
		}
		for(j=0;j<nblks;j++)
		{	
			for(k=0;k<npages;k++)
			{
				if(write(nand,"0",1));
				for(x=0;x<64;x++) if(write(nand,"1",1));
				for(x=0;x<8;x++) if(write(nand,"1",1));
				if(write(nand," ",1));
			}
			if(write(nand,"\n",1));
		}
		close(nand);
	}
	//printf("init: %d banks, %d blocks, %d pages per block\n",nbanks,nblks,npages);
	free(name);
	free(num);
	return 0;
}

int nand_write(int bank, int blk, int page, u32 *data, u32 spare)
{
	// write "data" and "spare" into the NAND flash memory pointed to by "blk" and "page"
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	int i;
	if(bank>=Banknum || bank<0)
	{
		printf("write(%d,%d,%d): failed, invalid bank number\n",bank,blk,page);
		return -1;
	}
	if(blk<0 || blk>=BPB)
	{
		printf("write(%d,%d,%d): failed, invalid block number\n",bank,blk,page);
		return -1;
	}
	if(page<0 || page>=PPB)
	{
		printf("write(%d,%d,%d): failed, invalid page number\n",bank,blk,page);
		return -1;
	}
	int nand;
	char* nbank = (char*)malloc(sizeof(char)*12);
	char* target = (char*)malloc(sizeof(char)*20);
	char* buffer = (char*)malloc(sizeof(char));
	sprintf(nbank,"%d",bank);
	strcpy(target,"bank_");
	strcat(target,nbank);
	nand = open(target,O_RDWR);
	lseek(nand,(DPB*blk)+(pgsize*page),SEEK_SET);
	if(read(nand,buffer,1));
	if(*buffer == '1')
	{
		printf("write(%d,%d,%d): failed, the page was already written\n",bank,blk,page);
		free(nbank);
		free(target);		
		free(buffer);
		close(nand);
		return -1;
	}
	for(i=0;i<page;i++)
	{
		lseek(nand, (DPB*blk)+(pgsize*i),SEEK_SET);
		if(read(nand,buffer,1));
		if(*buffer == '0')

		{

			printf("write(%d,%d,%d): failed, the page is not being sequentially written\n",bank,blk,page);
			free(nbank);
			free(target);
			free(buffer);
			close(nand);
			return -1;
		}
	}
	lseek(nand, (DPB*blk)+(pgsize*page),SEEK_SET);
	if(write(nand,"1",1));
	char *nspare = (char*)malloc(sizeof(char)*9);
	char *ndata = (char*)malloc(sizeof(char)*9);
	for(i=0;i<8;i++)
	{
		sprintf(ndata,"%8.8x",data[i]);
		if(write(nand,ndata,8));
	}
	sprintf(nspare,"%8.8x",spare);
	if(write(nand,nspare,8));
	//printf("write(%d,%d,%d): data = 0x%s, spare = 0x%s\n",bank,blk,page,ndata,nspare);
	free(nbank);
	free(target);
	free(ndata);
	free(nspare);
	close(nand);
	free(buffer);
	return 0;
}


int nand_read(int bank, int blk, int page, u32 *data, u32 *spare)
{
	// read "data" and "spare" from the NAND flash memory pointed to by "blk" and "page"
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	int k;
	if(bank>=Banknum || bank<0)
	{
		printf("read(%d,%d,%d): failed, invalid bank number\n",bank,blk,page);
		return -1;
	}
	if(blk<0 || blk>=BPB)
	{
		printf("read(%d,%d,%d): failed, invalid block number\n",bank,blk,page);
		return -1;
	}
	if(page<0 || page>=PPB)
	{
		printf("read(%d,%d,%d): failed, invalid page number\n",bank,blk,page);
		return -1;
	}
	int nand;
	char* nbank = (char*)malloc(sizeof(char)*12);
	char* target = (char*)malloc(sizeof(char)*20);
	char* buffer = (char*)malloc(sizeof(char));
	sprintf(nbank,"%d",bank);
	strcpy(target,"bank_");
	strcat(target,nbank);
	nand = open(target,O_RDONLY);
	lseek(nand,(DPB*blk)+(pgsize*page),SEEK_SET);
	if(read(nand,buffer,1));
	if(*buffer == '0')
	{
		printf("read(%d,%d,%d): failed, trying to read an empty page\n",bank,blk,page);
		free(nbank);
		free(target);
		free(buffer);
		close(nand);
		return -1;
	}
	char* rdata = (char*)malloc(sizeof(char)*8);
	char* rspare = (char*)malloc(sizeof(char)*8);	
	for(k=0;k<8;k++)
	{
		if(read(nand,rdata,8));
		data[k] = strtoul(rdata,NULL,16);	
	}
	if(read(nand,rspare,8));
	*spare = strtoul(rspare,NULL,16);
	//printf("read(%d,%d,%d): data = 0x%s, spare = 0x%s\n",bank,blk,page,rdata,rspare);
	free(nbank);
	free(target);
	free(rdata);
	free(rspare);
	free(buffer);
	close(nand);
	return 0;
}

int nand_erase(int bank, int blk)
{
	int i,x;
	char* data_d= (char*)malloc(sizeof(char)*(PPB+1));
	if(bank>=Banknum || bank<0)
	{
		printf("erase(%d,%d): failed, invalid bank number\n",bank,blk);
		return -1;
	}
	if(blk<0 || blk>=BPB)
	{
		printf("erase(%d,%d): failed, invalid block number\n",bank,blk);
		return -1;
	}
	int nand;
	char* nbank = (char*)malloc(sizeof(char)*12);
	char* target = (char*)malloc(sizeof(char)*20);
	char* buffer = (char*)malloc(sizeof(char));
	sprintf(nbank,"%d",bank);
	strcpy(target,"bank_");
	strcat(target,nbank);
	nand = open(target,O_RDWR,0777);
	for(i=0;i<PPB;i++)
	{
		lseek(nand, (DPB*blk)+(pgsize*i),SEEK_SET);
		if(read(nand,buffer,1));
		if(*buffer == '1') break;
	}
	if(i == PPB)
	{
		printf("erase(%d,%d): failed, trying to erase a free block\n",bank,blk);
		close(nand);
		free(nbank);
		free(buffer);
		free(target);
		return -1;
	}
	lseek(nand,(DPB*blk),SEEK_SET);
	for(i=0;i<PPB;i++)
	{	
		if(write(nand,"0",1));
		for(x=0;x<64;x++) if(write(nand,"1",1));
		for(x=0;x<8;x++) if(write(nand,"1",1));
		if(write(nand," ",1));
	}
	// erase the NAND flash memory block "blk"
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	//printf("erase(%d,%d): block erased\n",bank,blk);
	free(nbank);
	free(target);
	free(data_d);
	close(nand);
	free(buffer);
	return 0;
}


int nand_blkdump(int bank, int blk)
{
	if(bank>=Banknum || bank<0)
	{
		printf("blkdump(%d,%d): failed, invalid bank number\n",bank,blk);
		return -1;
	}
	if(blk<0 || blk>=BPB)
	{
		printf("blkdump(%d,%d): failed, invalid block number\n",bank,blk);
		return -1;
	}
	int count = 0;
	int i;
	int nand;
	char* nbank = (char*)malloc(sizeof(char)*12);
	char* target = (char*)malloc(sizeof(char)*20);
	char* buffer = (char*)malloc(sizeof(char));
	sprintf(nbank,"%d",bank);
	strcpy(target,"bank_");
	strcat(target,nbank);
	nand = open(target,O_RDWR,0777);
	for(i=0;i<PPB;i++)
	{
		lseek(nand, (DPB*blk)+(pgsize*i),SEEK_SET);
		if(read(nand,buffer,1));
		if(*buffer == '1') count++;
	}
	if(count == 0)
	{
		//printf("blkdump(%d,%d): FREE\n",bank,blk);
		free(nbank);
		free(target);
		free(buffer);
		close(nand);
		return 0;
	}
	char* rdata = (char*)malloc(sizeof(char)*9);
	char* rspare = (char*)malloc(sizeof(char)*9);
	//printf("blkdump(%d,%d): Total %d page(s) written\n",bank,blk,count);
	for(i=0;i<PPB;i++)
	{
		lseek(nand, (DPB*blk)+(pgsize*i),SEEK_SET);
		if(read(nand,buffer,1));
		if(*buffer == '1')
		{
			if(read(nand,rdata,8));
			lseek(nand,56,SEEK_CUR);
			if(read(nand,rspare,8));
			strcat(rdata,"\0");
			strcat(rspare,"\0");
			//printf("blkdump(%d,%d,%d): data = 0x%s, spare = 0x%s\n",bank,blk,i,rdata,rspare);
		}
	}
	// dump the contents of the NAND flash memory block [blk] (for debugging purpose)
	// returns 0 on success
	// returns -1 on errors with printing appropriate error message
	free(rdata);
	free(rspare);
	free(nbank);
	free(target);
	free(buffer);
	return 0;
}

