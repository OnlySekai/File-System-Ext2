#include<math.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<stdint.h>
#include<sys/stat.h>
#include<string.h>
#include"struct.h"
#include"util.h"

int image;
int fd;
const char *infile;
double r;//	inode/data_block

long bs;		//BlockSize
long bc;		//BlockCount
long dc;    //DataCount
long ds;    //DataStart
long is;    //InodeStart
long ic;		//InodeCount
long ib;		//InodeBlockCount
long bmpis;	//BitmapInodeStart
long bmpic;	//BitmapInodeCount
long bmpds; //BitmapBlockStart
long bmpdc;	//BitmapBlockCount
long tr;		//Trash

int main(int argc, char **argv)
{
	printf("super_fs = %i\ninode = %i\n", (int)sizeof(struct super_fs), (int)sizeof(struct inode));
	if(argc<2)
	{
		printf("Usage %s: <file> [options]\n", argv[0]);
		exit(0);
	}
	fd=open(argv[1], O_RDWR);
	if(fd<0)
	{
		perror("open()");
		exit(-1);
	}

	for(int i=2;i<argc;++i)
	{
		char *str1, *str2;
		str1=strtok(argv[i],"=");
		str2=strtok(NULL, "");
		if(!str1||!str2||strtok(NULL, ""))
		{
			printf("in argv[%i]=\"%s\": bad format\n", i, argv[i]);
			exit(-1);
		}

		if(!strcmp(str1, "inode")) ic=(uint32_t)atol(str2);
		else if(!strcmp(str1, "ratio")) r=(float)strtod(str2,NULL);
		else
		{
			printf("in argv[%i]=\"%s\": option not found\n", i, argv[i]);
			exit(-1);
		}
	}
	if(ic&&r)
	{
		printf("bad option: ratio and inode both set\n");
		exit(-1);
	}

	struct stat fs;
	if(fstat(fd, &fs)<0)
	{
		perror("fstat()");
		exit(-1);
	}

	bs=1024;
	bc=fs.st_size/bs;
	if(!r&&!ic) r=0.25;
	if(ic)
	{
		bmpic=(long)ceil((double)ic/8/bs);
		ib=(long)ceil(128.0*ic/bs);
		dc=floor((double)(bc-1-bmpic-ib)*8*bs/(8*bs+1));
		while(dc>0)
		{
			bmpdc=(double)ceil((double)dc/8/bs);
			if(bmpdc+dc+ib+bmpic+1<=bc)break;
			--dc;
		}
		if(dc<=0)
		{
			printf("Too many inode\n");
			exit(-1);
		}
	}
	else	//r!=0
	{
		ic=(long)floor((double)(bc-1)/(1.0/8/bs+128.0/bs+1.0/8/r/bs+1/r));
		while(ic>0)
		{
			dc=(long)ceil(ic/r);
			ib=(long)ceil(128.0*ic/bs);
			bmpic=(long)ceil((double)ic/8/bs);
			bmpdc=(long)ceil((double)dc/8/bs);
			if(dc+ib+bmpic+bmpdc+1<=bc)break;
			--ic;
		}
		if(ic<=0||dc<=0)
		{
			printf("Bad ratio\n");
			exit(-1);
		}
      dc=floor((double)(bc-1-bmpic-ib)*8*bs/(8*bs+1));
		while(dc>0)
		{
			bmpdc=(double)ceil((double)dc/8/bs);
			if(bmpdc+dc+ib+bmpic+1<=bc)break;
			--dc;
		}
	}

	tr=bmpic*bs-(long)ceil(ic/8.0)+bmpdc*bs-(long)ceil(dc/8.0);
	r=(double)ic/dc;

	printf(
			"block size: %li bytes, block count: %li blocks\n"
			"inode count: %li inodes, data block count: %li blocks\n"
			"inode/data: %lf, trash: %li bytes\n",
			bs, bc, ic, dc, r, tr);

	printf("Type 'yes' to format: ");
	char buf[8];
	scanf("%7s", buf);
	if(!strcmp(buf, "yes"))
	{
		image=fd;
      bmpis=1;
		bmpds=bmpis+bmpic;
		is=bmpds+bmpdc;
		ds=is+ib;
		printf(
				"Super block:\t\t0 - 0\n"
				"Inode bitmap:\t\t%li - %li\n"
				"Data block bitmap:\t%li - %li\n"
				"Inode table:\t\t%li - %li\n"
				"Data block:\t\t%li - %li\n",
				bmpis, bmpds-1, bmpds, is-1, is, ds-1,
				ds, ds+dc-1);
		super.magic=SUPER_MAGIC;
		super.block_count=bc;
		super.data_count=dc;
		super.inode_count=ic;
		super.block_bitmap=bmpds;
		super.inode_bitmap=bmpis;
		super.data_start=ds;
		super.inode_start=is;
		super.inode_free=ic;
		super.data_free=dc;
		super.last_alloced_block=-1;
		super.last_alloced_inode=-1;
		lseek(image, 0, SEEK_SET);
		write(image, &super, 1024);
		char zero[1024];
		memset(zero, 0, 1024);
		for(int i=0;i<bmpdc+bmpic;++i)
			write(image, zero, 1024);
		
		cache_init(image);
		uint32_t root=inode_alloc();
		if(root!=0)
		{
			printf("inode_alloc() return bad value\n");
			exit(-1);
		}

		struct inode *i=get_inode(root);
		inode_init(i, MODE_DIR);
		i->size=128; //1 dir entry lost+found
		inode_expand(root, 1);
		struct dir_entry *ent=get_file_block(0, i->data);
		ent[0].inode=inode_alloc();
		strcpy(ent[0].name, "lost+found");
		
		uint32_t k=ent[0].inode;
		i=get_inode(k);
		inode_init(i, MODE_DIR);
		i->size=256;
		inode_expand(k, 1);
		ent=get_file_block(0, i->data);
		ent[0].inode=k;
		strcpy(ent[0].name,".");
		ent[1].inode=root;
		strcpy(ent[1].name,"..");
		lseek(image, 0, SEEK_SET);
		write(image, &super, 1024);
	}
	else printf("Not format, disk remain the same\n");
	exit(0);
}
