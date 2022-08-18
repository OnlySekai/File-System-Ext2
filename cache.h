#pragma once
#include"struct.h" 
extern struct super_fs super;
struct inode *get_inode(uint32_t ino);
void *get_block(uint32_t bno);
void *get_bmp(uint32_t bno);
void cache_flush();
void cache_init(int fd);
 
