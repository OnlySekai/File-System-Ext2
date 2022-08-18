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


int rmf(char *path){


    uint32_t         count;
    char         dir[15][124];
    u_int32_t       
      p_ino,ch_ino;
    if (duyet_path(path,&count,dir,&p_ino,&ch_ino)==-1)
        return -1;
    if (ch_ino==-1) {
        printf("%s: khong ton tai\n",dir[count]);
        return -1;
    }
    //xoa
    struct  inode       *ch_inode=get_inode(ch_ino);
    if (ch_inode->mode!=MODE_NOR){
        printf("%s: khon phai file\n",dir[count]);
        return -1;
    }
    rm_inode(p_ino,ch_ino);
    printf("xoa file %s complete \n",dir[count]);
    return 1;

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
	rmf(argv[2]);
}