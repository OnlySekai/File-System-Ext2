#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include"struct.h"
#include"util.h"

size_t len;
char buffer[1024];

int image;
int list(uint32_t par)
{
	long ed=get_inode(par)->size/128;
	if(ed==0)return 1;
	for(long i=0;i<ed;++i)
	{
		const char *name=get_dir_entry(par, i)->name;
		printf("%s", buffer);
		printf("%s", (i==ed-1?"└":"├"));
		printf("──%s\n", get_dir_entry(par, i)->name);
		
		uint32_t chl=get_dir_entry(par, i)->inode;
		if(get_inode(chl)->mode==MODE_DIR)
		{
			const char a[]="│  ";
			const char b[]="   ";
			if(!strcmp(name, "."))continue;
			if(!strcmp(name, ".."))continue;
			strcpy(buffer+len, (i==ed-1?b:a));
			len+=strlen((i==ed-1?b:a));
			list(chl);
			len-=strlen((i==ed-1?b:a));
			buffer[len]=0;
		}
	}
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
	cache_init(image);//Chi can dong nay thoi
	//tree("");
	uint32_t ino=traverse_path(0, argv[2]);
	if(ino!=ino_error)
	{
		printf("%s\n", ino==0?"/":".");
		list(ino);
	}
}
