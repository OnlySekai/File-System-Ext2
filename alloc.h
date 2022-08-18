#pragma once
#include<stdint.h> 
#include"struct.h"
#include"cache.h"
uint32_t data_alloc();
uint32_t inode_alloc();
void free_data_cell(uint32_t bno);
void free_inode_cell(uint32_t ino);    
