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
#include"struct.h"
#include"util.h"
#include"libtuan.h"

char c_path[1000] = "/"; //for cd in future
void parse_path(char *path, uint32_t *count, char dir[15][124])
{
    *count = 0;
    int i = 0;
    if(path[0]!='/')
	 {
		 strcpy(c_path+1, path);
		 strcpy(path, c_path);
		}

	char *buf = path;
    while (buf[0]!='\0')
    {

        if (buf[0] != '/')
        {
            dir[*count][i] = buf[0];
            i++;
        }
        else
        {

            dir[*count][i] = 0;
            *count += 1;
            i = 0;
            //while(buf[0]==' ') buf++;
        }
        buf++;
    }
    dir[*count][i] = 0;
}

int duyet_path(char *path,uint32_t *count,char dir[15][124],u_int32_t *p_id,u_int32_t *ch_id)
{
    *p_id=0;
    parse_path(path,count,dir);
    for(int i=1;i<*count;i++){
        *p_id=dir_find_name(*p_id,dir[i]);
        if (get_inode(*p_id)->mode==MODE_NOR){
            printf("%s: khong phai folder\n",dir[i]);
            return -1;
        }
        if(*p_id==-1) {
            printf("%s:folder khong ton tai\n",dir[i]);
            return -1;
    }
    }
    if (ch_id!=NULL) *ch_id=dir_find_name(*p_id,dir[*count]);
    return 1;
}

uint32_t round_up(uint32_t a,uint32_t b){
    uint32_t temp=a/b;
    if (a!=(temp*b)) 
        return temp+1;
    else 
        return temp;
}
void rm0(uint32_t offset,uint32_t *t){
    uint32_t    *mp=t;
    for (uint32_t i = 0; i < offset; i++){
        free_data_cell(mp[i]);
    }
}

/*
    xoa bl lv1: la cac block block data
    offset so bl lv 0
    t:bitmap
*/

void rm1(uint32_t offset, uint32_t *t){
    uint32_t    *mp=t;
    uint32_t    blv1=round_up(offset,256);
    for (uint32_t i = 0; i < blv1; i++)
    {
        uint32_t  *bl=get_block(mp[i]);
        uint32_t    bl_fake[256];
        memcpy(bl_fake, bl, 1024);
        if (i!=(blv1-1))
            rm0(256,bl_fake);
        else
            rm0(offset-i*256,bl_fake);
    }
    rm0(blv1,mp);
}
/*
    xoa bl lv2: la cac block block data
    offset so bl lv 0
    t:bitmap
*/
void rm2(uint32_t offset, uint32_t *t){
    uint32_t    *mp=t;
    uint32_t    blv1=round_up(offset,256);
    uint32_t    blv2=round_up(blv1,256);
    for (uint32_t i = 0; i < blv2; i++)
    {
        uint32_t  *bl=get_block(mp[i]);
        uint32_t    bl_fake[256];
        memcpy(bl_fake, bl, 1024);
        if (i!=(blv2-1))
            rm1(256*256,bl_fake);
        else
            rm1(offset-i*256*256,bl_fake);
    }
    rm0(blv2,mp);
}


/*
    xoa bl lv3: la cac block block data
    offset so bl lv 0
    t:bitmap
*/
void rm3(uint32_t offset, uint32_t *t){
    uint32_t    *mp=t;
    uint32_t    blv1=round_up(offset,256);
    uint32_t    blv2=round_up(blv1,256);
    uint32_t    blv3=round_up(blv2,256);
    for (uint32_t i = 0; i < blv3; i++)
    {
        uint32_t  *bl=get_block(mp[i]);
        uint32_t   bl_fake[256];
        memcpy(bl_fake, bl, 1024);
        if (i!=blv3-1)
            rm2(256*256*256,bl_fake);
        else
            rm2(offset-i*256*256*256,bl_fake);

    }
    rm0(blv3,mp);
}

void rm_inode(u_int32_t p_ino,u_int32_t ch_ino){
    struct inode            ch_inode=*get_inode(ch_ino);
    //remove file:
    u_int32_t               bl=ch_inode.blocks;
    char                    lv=0;
    if (bl>12)              lv=1;
    if (bl>256+12)          lv=2;
    if (bl>256*256+256+12)  lv=3;
    uint32_t                del=bl;
    switch (lv)
    {
    case 3:
        del=bl-(256*256+256+12);
        rm3(del,ch_inode.data +14);
        bl-=del;
    case 2:
        del=bl-256-12;
        rm2(del,ch_inode.data+13);
        bl-=del;
    case 1:
        del=bl-12;
        rm1(del,ch_inode.data+12);
        bl-=del;
    default:
        del=bl-0;
        rm0(del,ch_inode.data);
        del-=bl;
    }
    
	 free_inode_cell(ch_ino);

    //config dir//ghi de dir cuoi vao dir can rm
    (get_inode(p_ino)->size)-=128;
	 uint32_t last=get_inode(p_ino)->size/128;
    struct dir_entry    dir_ent_las=*get_dir_entry(p_ino,last);
    struct dir_entry    *dir_ent_rm;
	 for(int i=0;i<=last;++i)
	 {
		 dir_ent_rm=get_dir_entry(p_ino, i);
		 if(dir_ent_rm->inode==ch_ino)goto founded;
	}
	 fprintf(stderr, "cant find ch_ino\n");
	
founded:
    *dir_ent_rm=dir_ent_las;
    if (get_inode(p_ino)->size%1024==0)
        inode_shrink(p_ino);
    get_inode(p_ino)->mtime=mytime();
}
