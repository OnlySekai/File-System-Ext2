#include<unistd.h>
#include<stdlib.h>
#include"struct.h"
#include"cache.h"
typedef struct {char d[1024];} array1024;

struct inode inode_cache[256];
array1024 block_cache[128];
array1024 bmp_cache[16];

uint32_t inode_tag[32];
uint32_t block_tag[128];
uint32_t bmp_tag[16];
#define notag ((uint32_t)(-1))
static int image;
struct super_fs super;

void map_bmp(uint32_t bno, uint32_t tag, uint32_t cel)
{
	size_t pos;
	pos=bno*1024;
	lseek(image, pos, SEEK_SET);
	read(image, bmp_cache+cel, 1024);
	bmp_tag[cel]=tag;
}

void map_inode(uint32_t ino, uint32_t tag, uint32_t cel)
{
	size_t pos;
	pos=(super.inode_start+ino/8)*1024;
	lseek(image, pos, SEEK_SET);
	read(image, inode_cache+(cel*8), 1024);
	inode_tag[cel]=tag;
}

void map_block(uint32_t bno, uint32_t tag, uint32_t cel)
{
	size_t pos;
	pos=(super.data_start+bno)*1024;
	lseek(image, pos, SEEK_SET);
	read(image, block_cache+cel, 1024);
	block_tag[cel]=tag;
}

void unmap_bmp(uint32_t cel)
{
	size_t pos;
	pos=(cel+bmp_tag[cel]*16)*1024;
	lseek(image, pos, SEEK_SET);
	write(image, bmp_cache+cel, 1024);
	bmp_tag[cel]=notag;
}

void unmap_inode(uint32_t cel)
{
	size_t pos;
	pos=(super.inode_start+(cel+inode_tag[cel]*32))*1024;
	lseek(image, pos, SEEK_SET);
	write(image, inode_cache+(cel*8), 1024);
	inode_tag[cel]=notag;
}

void unmap_block(uint32_t cel)
{
	size_t pos;
	pos=(super.data_start+(cel+block_tag[cel]*128))*1024;
	lseek(image, pos, SEEK_SET);
	write(image, block_cache+cel, 1024);
	block_tag[cel]=notag;
}

void *get_block(uint32_t bno)
{
	uint32_t tag, cel;
	tag=bno/128;
	cel=bno%128;
	if(block_tag[cel]==notag) map_block(bno, tag, cel);
	else if(block_tag[cel]!=tag)
	{
		unmap_block(cel);
		map_block(bno, tag, cel);
	}
	//Khong thay doi, cache hit
	return &block_cache[cel];
}

struct inode *get_inode(uint32_t ino)
{
	uint32_t tag, cel, w;
	tag=ino/256;
	cel=(ino%256)/8;
	w=ino%8;
	if(inode_tag[cel]==notag) map_inode(ino, tag, cel);
	else if(inode_tag[cel]!=tag)
	{
		unmap_inode(cel);
		map_inode(ino, tag, cel);
	}
	return inode_cache+(cel*8+w);
}

void *get_bmp(uint32_t bno)
{
	uint32_t tag, cel;
	tag=bno/16;
	cel=bno%16;
	if(bmp_tag[cel]==notag) map_bmp(bno, tag, cel);
	else
	{
		unmap_bmp(cel);
		map_bmp(bno, tag, cel);
	}
	return bmp_cache+cel;
}

void cache_init(int fd)
{
	image=fd;
	off_t old=lseek(fd, 0, SEEK_CUR);
	lseek(fd, 0, SEEK_SET);
	read(fd, &super, 1024);
	lseek(fd, old, SEEK_SET);
	for(uint32_t i=0;i<32;++i)
		inode_tag[i]=notag;
	for(uint32_t i=0;i<128;++i)
		block_tag[i]=notag;
	for(uint32_t i=0;i<16;++i)
		bmp_tag[i]=notag;
	atexit(cache_flush);
}

void cache_flush()
{
	for(uint32_t i=0;i<32;++i)
		if(inode_tag[i]!=notag)unmap_inode(i);
	for(uint32_t i=0;i<128;++i)
		if(block_tag[i]!=notag)unmap_block(i); 
	for(uint32_t i=0;i<16;++i)
		if(bmp_tag[i]!=notag)unmap_bmp(i); 
	off_t old=lseek(image, 0, SEEK_CUR);
 	lseek(image, 0, SEEK_SET);
	write(image, &super, 1024);
	lseek(image, old, SEEK_SET);
}


