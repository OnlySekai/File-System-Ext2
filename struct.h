#pragma once
#include<stdint.h>
#define SUPER_MAGIC 0xEF53
#define ino_error (uint32_t)(-1)

extern int ERROR; 
enum
{
	MODE_NOR=0,
	MODE_DIR=1,
	MODE_LIK=2
};

struct dir_entry
{
	uint32_t inode;
	char name[124];  //counted null
};

struct super_fs
{
	uint64_t magic;
	uint32_t block_count;
	uint32_t data_count;
	uint32_t inode_count;
   uint32_t block_bitmap;
	uint32_t inode_bitmap;
	uint32_t data_start;
	uint32_t inode_start;
	uint32_t inode_free;
	uint32_t data_free;
	uint32_t last_alloced_block;
	uint32_t last_alloced_inode;
	char pad[976];				//TODO
};

struct inode 
{
	uint32_t link_count;
	uint32_t flag;
	uint32_t atime;         //Later
	uint32_t ctime;			//Later
	uint32_t mtime;         //Later
	uint32_t size;          //TODO
	uint32_t blocks;        //TODO
	uint32_t data[15];      //TODO: chinh tron 128byte
	uint8_t mode;				//Dir or file
	char pad[39];
};

