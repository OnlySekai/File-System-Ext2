#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include"struct.h"
#include"util.h"

int image;

int mkfile(const char *path, const char *new)
{
	uint32_t par=traverse_path(0, path);
	if(par==(uint32_t)(-1))
		return 0;
   if(dir_find_name(par, new)!=(uint32_t)(-1))
	{
		fprintf(stderr ,"trung ten\n");
		return 0;
	}
	printf("making file\n");

	uint32_t chl= inode_alloc();
	struct inode *i=inode_get(chl);
	inode_init(i, MODE_NOR);

	struct inode *father=get_inode(par);
	if(father->size%1024==0) inode_expand(par,1);
	struct dir_entry *ent=get_dir_entry(par, father->size/128);
	strcpy(ent->name, new);
	ent->inode=chl;
	father->size+=128;
	father->mtime=mytime();
	return 1;
}

   
int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("Usage %s: <file>", argv[0]);
		exit(0);
	}
	image=open(argv[1], O_RDWR);
	if(image<0)
	{
		perror("open()");
		exit(-1);
	}
	cache_init(image);//Chi can dong nay thoi
	mkfile("", "fil");

	for(int i=0;i<get_inode(0)->size/128;++i)
		printf("%s\n", get_dir_entry(0, i)->name);
}
