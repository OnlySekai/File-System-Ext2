#pragma once
#include"struct.h"
#include"cache.h"
#include"alloc.h"

uint32_t traverse_path(uint32_t ino, const char *const_path);
void inode_init(struct inode *i, uint16_t mode);
uint32_t mytime();
struct dir_entry *get_dir_entry(uint32_t ino, uint32_t c);
int copy_file_to_blank_inode(uint32_t src_ino, uint32_t des_ino);
int remove_dir_entry(uint32_t ino_dir, uint32_t offset);
int add_dir_entry(uint32_t ino_dir, uint32_t ino_chl, const char *name);
uint32_t inode_dir_alloc(uint32_t ino_par);
uint32_t dir_find_name(uint32_t ino, const char *name);
uint32_t inode_expand(uint32_t ino, uint32_t count);
uint32_t translate_file_block(uint32_t bno, uint32_t t[15]);
uint32_t count_all_block(uint32_t ino);
void *get_file_block(uint32_t bno, uint32_t mp[15]);
void inode_shrink(uint32_t ino);
void inode_free(uint32_t ino);
void tree_view(uint32_t ino);
uint32_t get_dir_size(uint32_t ino); 
void copy_file_nocheck(uint32_t srcino, uint32_t desino, const char *name);
void dir_usage(uint32_t ino, uint32_t *block, uint32_t *inode);
