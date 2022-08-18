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
size_t wr(uint32_t ino, size_t offset, void *buffer, size_t count)
{
	struct inode *p_ino=get_inode(ino);
	if(offset>p_ino->size)
	{
		printf("Offset to big, writing to end of file\n");
		offset=p_ino->size;
	}

	char *f=get_file_block(offset/1024, p_ino->data);
	for(uint32_t i=0;i<count;++i,++offset)
	{
		uint32_t blk=offset/1024;
		uint32_t off=offset%1024;
		if(off==0)
		{
			if(blk>=p_ino->blocks)
				if(!inode_expand(ino, 1))
				{
					count=i;
					break;
				}
			f=get_file_block(blk, p_ino->data);
		}
		f[off]=((char*)buffer)[i];
	}
	if(p_ino->size<offset)p_ino->size=offset;
	p_ino->mtime=mytime();
	return count;
}
   
int main(int argc, char **argv)
{
	if(argc < 3 || argc > 4)
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
	if(get_inode(ino)->mode!=MODE_NOR)
	{
		printf("%s is not a file\n", argv[2]);
		exit(-1);
	}
	get_inode(ino)->atime=mytime();
	int done=0;
	size_t offset=argc==4?strtoul(argv[3], NULL, 10):0;
	size_t total=0;
	for(;!done;)
	{
		char buf[1024];
		size_t ret=fread(buf, 1, 1024, stdin);
		if(ret<1024)
		{
			if(feof(stdin))done=1;
			if(ferror(stdin))
			{
				perror("fread()");
				exit(-1);
			}
		}
		size_t writed=wr(ino, offset, buf, ret);
		offset+=writed;
		total+=writed;
		if(writed<ret)break;
	}
	printf("Writed %lu bytes to file\n", total);
}
