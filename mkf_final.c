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
#include"util.h"
#include"libtuan.h"

int image;

int mkf(char *path){
    uint32_t         count;
    char         dir[15][124];
    uint32_t         c_ino;
    if (!super.inode_free) {
	    printf("khong du data hoac inode\n");
	    return -1;
    }

    if (duyet_path(path,&count,dir,&c_ino,NULL)==-1){
        return -1;
    };
    //check trung 
    if (dir_find_name(c_ino,dir[count])!=-1){
        printf("%s: da ton tai\n",dir[count]);
        return -1;
    }
    struct inode    *c_inode;
    //create dir_entry
    u_int32_t       fino=inode_alloc();
    struct inode    *finode=get_inode(fino);
	 inode_init(finode, MODE_NOR);
    //gan inode vua tao vao block
    c_inode=get_inode(c_ino);
    if (!(c_inode->size%1024))
        inode_expand(c_ino,1);
    struct dir_entry *pa_ch_dir;
    pa_ch_dir=get_dir_entry(c_ino,c_inode->size/128);
    strcpy(pa_ch_dir->name,dir[count]);
    pa_ch_dir->inode=fino;
    //cau hinh not parent
    c_inode->size+=128;
    c_inode->mtime=mytime();
    printf("%s: tao file complete\n",dir[count]);
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
	if(argc!=3)exit(-1);
	mkf(argv[2]);
}
