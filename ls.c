#include <stdio.h>
#include <stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<time.h>
#include<unistd.h>
#include "struct.h"
#include "util.h"
#include "cache.h"
#include "libtuan.h"
#include <stdint.h>
#include <string.h>

int image;

void viettime(char *time)
{
	time[strlen(time)-1]=0;
	printf("%-30s", time);
}
void ls(const char *p){
	char *path=strdupa(p);
	char dir[15][124];
	uint32_t count,par_ino,chl_ino;
	if(duyet_path(path,&count,dir,&par_ino,&chl_ino)==-1)
		exit(-1);
	if(par_ino!=0&&((get_inode(chl_ino)->mode!=MODE_DIR)||chl_ino==-1))
	{
		//TODO erro();
		printf("%s: khong phai la folder\n",dir[count]);
		exit(0);
	}
	if(chl_ino==-1)chl_ino=par_ino;
	printf("%-*s",4,"STT");
	printf("%-*s",5, "MODE");
	printf("%-*s",12, "SIZE");
	printf("%-*s",20, "NAME");
	printf("%-*s",30, "CREATED TIME");
	printf("%-*s",30, "MODDED TIME");
	printf("%-*s",30, "ACCESSED TIME");
	uint32_t num_dir=get_inode(chl_ino)->size/128;
	for (int i=0;i<num_dir;i++){
		putchar('\n');
		struct dir_entry dir_tmp=*get_dir_entry(chl_ino,i);
		printf("%-4d",i+1);
		struct inode inode_tmp=*get_inode(dir_tmp.inode);
		if (inode_tmp.mode==MODE_NOR)
		{
			printf("%-*s", 5, "FILE");
			printf("%-12u",inode_tmp.size);
		}
		else
		{
			printf("%-*s", 5, "DIR");
			printf("%-12u",get_dir_size(dir_tmp.inode));
		}
		printf("%-20s",dir_tmp.name);
		time_t t1=inode_tmp.ctime, t2=inode_tmp.mtime, t3=inode_tmp.atime;
		viettime(ctime(&t1));
		viettime(ctime(&t2));
		viettime(ctime(&t3));
	}
		putchar('\n');
}
int main(int argc,char **argv){
	image=open(argv[1],O_RDWR);
	if(image<0){
		perror("open()");
		exit(-1);
	}
	cache_init(image);
	ls(argv[2]);
}
		

