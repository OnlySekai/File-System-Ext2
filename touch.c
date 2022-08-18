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
int touch(uint32_t ino)
{
	struct inode *ent=get_inode(ino);
	ent->atime=mytime();
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
	touch(ino);
}
