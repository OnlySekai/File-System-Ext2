#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <malloc.h>
#include "struct.h"
#include"libtuan.h"
#include"util.h"

int image;
void rmd_inode(u_int32_t p_ino,u_int32_t ch_ino){
    struct  inode       ch_inode=*get_inode(ch_ino);
    if (ch_inode.size>128*2){
        uint32_t    dir_visted=0;
        u_int32_t   dir_max=ch_inode.size/128-2;
        while(dir_visted<dir_max){
            uint32_t i=-1;
            dir_need:
            i++;   
            struct  dir_entry    dir_entry=*get_dir_entry(ch_ino,i);
            if(!strcmp(dir_entry.name,".")) goto dir_need;
            if(!strcmp(dir_entry.name,"..")) goto dir_need;
            ch_inode=*get_inode(dir_entry.inode);
            dir_visted++;
            if (ch_inode.mode==MODE_NOR)
                rm_inode(ch_ino,dir_entry.inode);
            if(ch_inode.mode==MODE_DIR)
                rmd_inode(ch_ino,dir_entry.inode);
        }
    }
   rm_inode(p_ino,ch_ino);
}


int rmd(char *path){
    uint32_t        count;
    char         dir[15][124];
    u_int32_t         p_ino,ch_ino;
    if (duyet_path(path,&count,dir,&p_ino,&ch_ino)==-1)
        return -1;
    if (ch_ino==-1) {
        printf("%s: khong ton tai\n", dir[count]);
        return -1;
    }
    if((!strcmp(dir[count],"."))||(!strcmp(dir[count],".."))){
        //TODO erro();
        printf("%s: khong hop le\n",dir[count]);
        return -1;
    }
    //xoa
    struct  inode       *ch_inode=get_inode(ch_ino);
    if (ch_inode->mode!=MODE_DIR){
        printf("%s: khong phai la folder\n",dir[count]);
        return -1;
    }
    if (ch_inode->mode==MODE_DIR){
        rmd_inode(p_ino,ch_ino);
        return 1;
    }
    return -1;
}



int main(int argc, char **argv)
{
	image=open(argv[1], O_RDWR);
	if(image<0)
	{
		perror("open()");
		exit(-1);
	}
	cache_init(image);
   // char dir[15][124];
    //printf("%d",get_inode(0)->size);
 //   mkd("/abc");
  //  printf("%d",get_inode(0)->size);
	if(argc!=3)exit(-1);
	rmd(argv[2]);
}
