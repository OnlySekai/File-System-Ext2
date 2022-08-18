#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include"struct.h"
#include"util.h"

int image;
int stat(uint32_t ino, const char *name)
{
	struct inode *ent=get_inode(ino);
	printf("File: %s\n", name);
	printf("Type: %s\n", ent->mode==MODE_NOR?"normal file":"directory");
	printf("Inode:%14u\n", ino);
	printf("Size: %14u\tDatablock: %14u\tAllblock:%14u\n", ent->size, ent->blocks, count_all_block(ino));
	time_t time1=ent->ctime, time2=ent->atime, time3=ent->mtime;
	printf("Create time: \t%s", ctime(&time1));
	printf("Access time: \t%s", ctime(&time2));
	printf("Modified time: \t%s", ctime(&time3));
	return 1;
}
   
int main(int argc, char **argv)
{
	if(argc != 3)
	{
		printf("Usage %s: <disk image> <path>\n", argv[0]);
		exit(0);
	}
	image=open(argv[1], O_RDWR);
	if(image<0)
	{
		perror("open()");
		exit(-1);
	}
	cache_init(image);
	uint32_t ino=traverse_path(0, argv[2]);
	if(ino==-1) exit(-1);
	stat(ino, basename(argv[2]));
}
