
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
int copy_file(uint32_t srcino, uint32_t desino, const char *name)
{
	if(get_inode(desino)->mode!=MODE_DIR)
	{
		printf("Not a dir\n");
		return 0;
	}
	uint32_t blks=count_all_block(srcino)+(get_inode(desino)->size%1024==0);
	if(super.data_free<blks)
	{
		printf("Not enough free data\n");
		return 0;
	}
	if(!super.inode_free)
	{
		printf("Not enough free inode\n");
		return 0;
	}	
	if(dir_find_name(desino, name)!=-1)
	{
		printf("File existed\n");
		return 0;
	}
	copy_file_nocheck(srcino, desino, name);
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
	if(get_inode(srcino)->mode!=MODE_NOR)
	{
		printf("copy file only\n");
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
	return copy_file(srcino, desino, new_name);
}
