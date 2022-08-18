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
size_t rd(uint32_t ino, size_t offset, void *buffer, size_t count)
{
	struct inode *p_ino=get_inode(ino);
	char *f=get_file_block(offset/1024, p_ino->data);
	for(uint32_t i=0;i<count;++i,++offset)
	{
		if(offset>=p_ino->size)return i;
		uint32_t blk=offset/1024;
		uint32_t off=offset%1024;
		if(off==0)f=get_file_block(blk, p_ino->data);
		((char*)buffer)[i]=f[off];
	}
	return count;
}
   
int main(int argc, char **argv)
{
	if(argc < 3 || argc > 5)
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
	size_t offset=argc>=4?strtoul(argv[3], NULL, 10):0;
	size_t bytetoread=argc==5?strtoul(argv[4], NULL, 10):-1;
	size_t total=0;
	get_inode(ino)->atime=mytime();
	for(;total<bytetoread;)
	{
		char buf[1024];
		uint32_t rr=bytetoread-total;
		if(rr>1024)rr=1024;
		size_t readed=rd(ino, offset, buf, rr);
		fwrite(buf, 1, readed, stdout);
		offset+=readed;
		total+=readed;
		if(readed<1024)break;
	}
	fprintf(stderr, "\nReaded %lu byte\n", total);
}
