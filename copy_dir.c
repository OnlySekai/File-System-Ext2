#include<libgen.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include"struct.h"
#include"util.h"

int image;
void copy_dir_nocheck(uint32_t srcino, uint32_t desino, const char *name)
{
	if(get_inode(srcino)->mode==MODE_NOR) 
		return copy_file_nocheck(srcino, desino, name);
	
	uint32_t newino=inode_dir_alloc(desino);
	uint32_t ed=get_inode(srcino)->size/128;
	for(uint32_t i=0; i<ed;++i)
	{
		//name poiter not safe so clone here
		struct dir_entry ent=*get_dir_entry(srcino,i);
		if(!strcmp(ent.name, "."))continue;
		if(!strcmp(ent.name, ".."))continue;
		copy_dir_nocheck(ent.inode, newino, ent.name);
	}
	//copy cha vao con ko bi chay vo han
	add_dir_entry(desino, newino, name);
}

int copy_dir(uint32_t srcino, uint32_t desino, const char *name)
{
	if(get_inode(desino)->mode!=MODE_DIR)
	{
		printf("Not a dir\n");
		return 0;
	}
	uint32_t blk=0, ind=0;
	dir_usage(srcino, &blk, &ind);
	blk+=(get_inode(desino)->size%1024==0);
	if(super.data_free<blk)
	{
		printf("Not enough free data\n");
		return 0;
	}
	if(super.inode_free<ind)
	{
		printf("Not enough free inode\n");
		return 0;
	}
	if(dir_find_name(desino, name)!=-1)
	{
		printf("File exsited\n");
		return 0;
	}
   copy_dir_nocheck(srcino, desino, name);
	return 1;
}
   
int main(int argc, char **argv)
{
	if(argc!=4)
	{
		printf("Usage %s: <disk> <file> <dest>\n", argv[0]);
		exit(0);
	}

	image=open(argv[1], O_RDWR);
	if(image<0)
	{
		perror("open()");
		exit(-1);
	}
	cache_init(image);
	
	uint32_t srcino=traverse_path(0, argv[2]);
   if(srcino==-1) exit(-1);
	if(get_inode(srcino)->mode!=MODE_DIR)
	{
		printf("copy file dir\n");
		exit(-1);
	}
	const char *new_name=NULL;
	uint32_t desino;
	if(argv[3][strlen(argv[3])-1]=='/')
	{
		new_name=basename(argv[2]);
		desino=traverse_path(0, argv[3]);
	}
	else 
	{
		new_name=basename(argv[3]);
		const char *dname=dirname(argv[3]);
		if(!strcmp(dname, "."))dname="";
		desino=traverse_path(0, dname);
	}
   if(desino==-1) exit(-1);
	if(get_inode(desino)->mode!=MODE_DIR)
	{
		printf("%s is not a folder\n", argv[3]);
		exit(-1);
	}
	return copy_dir(srcino, desino, new_name);
}
