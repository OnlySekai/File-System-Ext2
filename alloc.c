#include<stdlib.h>
#include<stdio.h>
#include"alloc.h"
void free_data_cell(uint32_t bno)
{
	size_t pos=super.block_bitmap + bno/(1<<13);
	bno%=1<<13;
   uint8_t *bmp=get_bmp(pos);
	if(!(bmp[bno/8]&(1<<(bno%8))))
		fprintf(stderr, "freeing a free data block\n");
	else 
	{
		bmp[bno/8]^=(1<<(bno%8));
		++super.data_free;
	}
}

void free_inode_cell(uint32_t ino)
{
 	size_t pos=super.inode_bitmap + ino/(1<<13);
	ino%=1<<13;
   uint8_t *bmp=get_bmp(pos);
	if(!(bmp[ino/8]&(1<<(ino%8))))
		fprintf(stderr, "freeing a free inode cell\n");
	else 
	{
		bmp[ino/8]^=(1<<(ino%8)); 
		++super.inode_free;
	}
}

uint32_t find_zero(uint32_t start, uint32_t len, uint32_t offset, uint32_t block_start)
{
	uint32_t end=start+len;
	uint32_t pivot=offset;
	uint32_t count=0;
	if(pivot<start)pivot=start;
	if(pivot>=end)pivot=start;
	uint64_t *b=get_bmp(block_start+pivot/8192);

	while(count<len)
	{
		uint32_t virtual=pivot%8192;
      if(virtual==0) b=get_bmp(block_start+pivot/8192);
		
		uint32_t u=virtual/64;
		uint32_t v=virtual%64;
      if(!(b[u]&((uint64_t)1<<v)))
		{
			b[u]|=(uint64_t)1<<v;
			return pivot;
		}

		++pivot;
		++count;	
		if(pivot==end)
		{
			pivot=start;
			b=get_bmp(block_start+pivot/8192);
		}
	}
	fprintf(stderr, "FF, free inode count, bitmap missmatch\n");
	exit(-1);
}

uint32_t data_alloc()
{
	if(super.data_free==0)
	{
		fprintf(stderr, "Out of free blocks\n");
		return -1;
	}
	--super.data_free;
	super.last_alloced_block=find_zero(
			0, super.data_count, 
			super.last_alloced_block+1, 
			super.block_bitmap);
	return super.last_alloced_block;
}   

uint32_t inode_alloc()
{
	if(super.inode_free==0)
	{
		fprintf(stderr, "Out of free inode\n");
		return -1;
	}
	--super.inode_free;
	super.last_alloced_inode=find_zero(
			0, super.inode_count,
			super.last_alloced_inode+1,
			super.inode_bitmap);
	return super.last_alloced_inode;
}
 
