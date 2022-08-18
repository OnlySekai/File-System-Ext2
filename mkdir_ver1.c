#include<string.h>
#include"struct.h"
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>

int image;

uint32_t mytime() {return 0;} //TODO

int mkdir(char *path, char *new_name){
	int count;
	int c_ino;
	char *name=strtok(path, "/");

	uint32_t par;
	par=0;
	if(name) while(1)
	{
		par=dir_find_name(par, name);
		if(get_inode(par)->mode==0)
		{
			fprintf(stderr, "%s is not a folder\n", name);
			return 0;
		}
		if(par==(uint32_t)(-1)) 
		{
			//TODO
			fprintf(stderr, "not found folder %s\n", name);
			return 0;
		}
		printf("inode:%u - name: %s\n", par, name);
		name=strtok(NULL, "/");
		if(!name) break;
	}

	if(dir_find_name(par, new_name)!=(uint32_t)(-1))
	{
		fprintf(stderr ,"trung ten\n");
		return 0;
	}
	printf("making dir\n");
	fflush(stdout);

	//TODO: cho thanh ham
   uint32_t chl=inode_alloc();
	struct inode *child=get_inode(chl);
	child->link_count=1;
	child->flag=0;
	child->atime=mytime();
	child->ctime=mytime();
	child->mtime=mytime();
	child->blocks=0;
	memset(child->data, -1, sizeof(child->data));
	child->size=256;
	child->mode=1;
	inode_expand(chl, 1);
	struct dir_entry *ent=get_file_block(0, child->data);
	ent[0].inode=chl;
	strcpy(ent[0].name,".");
	ent[1].inode=par;
	strcpy(ent[1].name,"..");

	struct inode *father=inode_get(par);
	if(father->size%1024==0)
		inode_expand(par, 1);
	
	ent=get_file_block(father->blocks-1, father->data);
	ent[(father->size%1024)/128].inode=chl;
	strcpy(ent[(father->size%1024)/128].name,new_name);
	father->size+=128;
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
	mkdir("/", "asu");
	mkdir("/", "ac");
	mkdir("/", "c");

	struct inode *i=get_inode(0);
	struct dir_entry *ent;
	printf("root size = %u\n", i->size);
	for(uint32_t k=0;k<i->size/128;++k)
	{
		ent=get_file_block(k/8,i->data);
		printf("%u: %s\n", k, ent[k%8].name);
	}
}
