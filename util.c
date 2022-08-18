#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"struct.h"
#include"util.h"

/*
 * Xuat phat tu ino di den const_path
 * return -1 neu loi
 * return ino dich neu thanh cong
 * ham su dung block va inode cache
 */
uint32_t traverse_path(uint32_t ino, const char *const_path)
{
	char *path=strdup(const_path);
   char *name=strtok(path, "/");
	uint32_t par=ino;
	if(name) while(1)
	{
		if(get_inode(par)->mode!=1)
		{
			printf("Not found %s\n", name);
			free(path);
			return ino_error;
		}
		if(strcmp(name,"."))
		{
			par=dir_find_name(par, name);
			if(par==(uint32_t)(-1))
			{
				printf("Not found %s\n", name);
				free(path);
				return ino_error;
			}
		}
		if(!(name=strtok(NULL, "/")))break;
	}
	free(path);
	return par;
}

/*
 * Khoi tao inode voi nhung gia tri mac dinh
 */
void inode_init(struct inode *i, uint16_t mode)
{
	i->mode=mode;
	i->link_count=1;
	i->flag=0;
	i->atime=mytime();
	i->ctime=mytime();
	i->mtime=mytime();
	i->blocks=0;
	i->size=0;
	memset(i->data, -1, sizeof(i->data));
}

/*
 * Tinh toan offset nhanh
 * Chu y ham su dung block cache
 */
struct dir_entry *get_dir_entry(uint32_t ino, uint32_t c)
{
	struct dir_entry *ent;
	ent=get_file_block(c/8, get_inode(ino)->data);
	return ent+(c%8);
}                  

uint32_t mytime()
{
	return time(NULL);
}

/*
 * Copy file (not dir or link)
 * Khong check inode->mode
 * Co check tran disk (failed->printf+return 0)
 * Success -> return 1
*/
int copy_file_to_blank_inode(uint32_t src_ino, uint32_t des_ino)
{
	struct inode src=*get_inode(src_ino);			//Since src_ino is read only
	if(super.data_free<src.blocks)
	{
		printf("Copy failed: not enough free space\n");
		return 0;
	}

	char buf[1024];										//Block may get swap out
	struct inode *ptr_des=get_inode(des_ino);		//Never got swap out
	inode_expand(des_ino, src.blocks);
	for(uint32_t i=0;i<src.blocks;++i)
	{
		memcpy(buf, get_file_block(i, src.data), 1024);
		memcpy(get_file_block(i, ptr_des->data), buf, 1024);
	}
	ptr_des->link_count=1;
	ptr_des->flag=src.flag;
	ptr_des->atime=mytime();
	ptr_des->mtime=mytime();
	ptr_des->size=src.size;
	ptr_des->mode=src.mode;
	return 1;
}

/*
 * Xoa 1 dir entry khoi dir inode
 * Khong check inode->mode
 * Luon luon return 1 tru khi code ngu
 */
int remove_dir_entry(uint32_t ino_dir, uint32_t offset)
{
	struct inode *i=get_inode(ino_dir);
	uint32_t last=i->size/128;
	if(last==0)
	{
		printf("Dir empty\n");
		return 0;
	}
	if(offset>=last)
	{
		printf("Dir offset bad\n");
		return 0;
	}

	struct dir_entry l=*get_dir_entry(ino_dir, last-1);
	struct dir_entry *dir_em=get_dir_entry(ino_dir, offset);
	*dir_em=l;
	i->size-=128;
	if(i->size%1024==0)
		inode_shrink(ino_dir);
	return 1;
}             

/*
 * Them mot dir entry vao inode dir
 * Khong check inode->mode
 * return 0 khi khong du block, name qua dai
 * return 1 khi thanh cong
 */
int add_dir_entry(uint32_t ino_dir, uint32_t ino_chl, const char *name)
{
	struct inode *par;
	struct dir_entry *lastdir;
	if(strlen(name)>sizeof(lastdir->name))
	{
		printf("Ten qua dai\n");
		return 0;
	}
	par=get_inode(ino_dir);
	if(par->size%1024==0)
		if(!inode_expand(ino_dir,1))
			return 0;
	lastdir=get_dir_entry(ino_dir, par->size/128);
	strcpy(lastdir->name, name);
	lastdir->inode=ino_chl;
	par->size+=128;
	return 1;
}

/*
 * Tao mot inode dang thu muc
 * return -1 neu het inode, hoac het block
 * return inode vua tao neu thanh cong
 */
uint32_t inode_dir_alloc(uint32_t ino_par)
{
	uint32_t ino_chl;
	struct inode *chl;
	struct dir_entry *dir;
	ino_chl=inode_alloc();
	if(ino_chl==-1)return -1;
	chl=get_inode(ino_chl);
	inode_init(chl, MODE_DIR);
	if(!inode_expand(ino_chl,1))
	{
		free_inode_cell(ino_chl);
		return -1;
	}
	chl->size=128*2;
	dir=get_dir_entry(ino_chl, 0);
	strcpy(dir[0].name, ".");
	strcpy(dir[1].name, "..");
	dir[0].inode=ino_chl;
	dir[1].inode=ino_par;
	return ino_chl;
}
    
/*
 * Tim dir ten la name trong thu muc
 * thu muc co inode ino
 * ko check inode->mode
 * su dung cache inode, cache block
 */
uint32_t dir_find_name(uint32_t ino, const char *name)
{
	struct inode *i;
	i=get_inode(ino);

	uint32_t dir=0;
	for(;dir<i->size/128;++dir)
	{
		struct dir_entry *ent=get_dir_entry(ino, dir);
		if(!strcmp(ent->name, name))
			return ent->inode;
	}
	return -1;
}

/* Backend cua inode_expand */
int expand(uint32_t *array, uint32_t offset, int dep)
{
	if(dep==0)
	{
		array[offset]=data_alloc();
		return array[offset]!=-1;
	}

	int m=(1<<(8*dep));
	uint32_t cel=offset/m;
	if(offset%m==0)
	{
		array[cel]=data_alloc();
		if(array[cel]==-1)return 0;
	}
	return expand(get_block(array[cel]), offset%m, dep-1);
}

/*
 * Mo rong map cua inode, cap nhat ->data, ->blocks
 * So luong block yeu cau = count
 * Tra ve so luong block da alloc, it hon nghia la loi
 */
uint32_t inode_expand(uint32_t ino, uint32_t count)
{
	if(count==0)return 0;
	struct inode *i=get_inode(ino);
	uint32_t tmp=i->blocks+count;
	if(i->blocks>tmp)
	{
		fprintf(stderr, "inode_expand size overflow\n"); 
		exit(-1);
	}
	for(uint32_t j=0;j<count;++j)
	{
		uint32_t bno=i->blocks;
		uint32_t *mp=i->data;
		int l=0;
		if(bno<12)
		{
			mp[bno]=data_alloc();
			if(mp[bno]==-1)return j;
			++(i->blocks);
			continue;
		}
		bno-=12;
		if(bno<256) goto layer;
		bno-=256;++l;
		if(bno<65536) goto layer;
		bno-=65536;++l;
		if(bno<(1<<24)) goto layer;
		fprintf(stderr, "Too much block\n");
		return j;

layer:               
		if(bno==0)
		{
			mp[12+l]=data_alloc();
			if(mp[12+l]==-1)return j;
		}
		if(!expand(get_block(mp[12+l]), bno, l))return j;
		++(i->blocks);
	}
	return count;
}
    
/*
 * Tu bang 15 con tro, tim block thu bno trong do
 * return -1 neu loi
 * return so block data vat ly
 */
uint32_t translate_file_block(uint32_t bno, uint32_t t[15])
{
	uint32_t *mp=t;
	if(bno<12) return mp[bno];
	bno-=12;
	if(bno<256)
	{
		mp=get_block(mp[12]);
		return mp[bno];
	}
	bno-=256;
	if(bno<65536)
	{
		mp=get_block(mp[13]);
		case2:
		mp=get_block(mp[bno/256]);
		bno%=256;
		return mp[bno];
	}
	bno-=65536;
	if(bno<1<<24)
	{
		mp=get_block(mp[14]);
		mp=get_block(mp[bno/65536]);
		bno%=65536;
      goto case2;
	}

	fprintf(stderr, "%s: bno out of range", __FUNCTION__);
	return (uint32_t)(-1);
}

/*
 * Nap block thu bno cua bang 15 con tro vao cache
 * Tuong duong get_block(translate_file_block)
 */
void *get_file_block(uint32_t bno, uint32_t t[15])
{
	return get_block(translate_file_block(bno, t));
}

/*
 * Backend cua inode_shrink
 */
int shrink_del(uint32_t bno, uint32_t offset, int depth)
{
	if(depth>-1)
	{
		uint32_t *a=get_block(bno);
		uint32_t m=1<<(8*depth);
		if(shrink_del(a[offset/m], offset%m, depth-1)&&offset==0)
		{
			free_data_cell(bno);
			return 1;
		}
		return 0;
	}
	else 
	{
		free_data_cell(bno);
		return 1;
	}
}

/*
 * Giam block quan ly cua inode di 1
 * yen tam ko loi :>>
 */
void inode_shrink(uint32_t ino)
{
	struct inode *i=get_inode(ino);
	if(i->blocks==0)return;
	uint32_t offset=--(i->blocks);
	if(offset<12)
	{
		free_data_cell(i->data[offset]);
		i->data[offset]=-1;
		return;
	}
	offset-=12;
	if(offset<256)
	{
		if(shrink_del(i->data[12], offset, 0))
			i->data[12]=-1;
		return;
	}
	offset-=256;
	if(offset<65536)
	{
		if(shrink_del(i->data[13], offset, 1))
			i->data[13]=-1;
		return;
	}
	offset-=65536;
   if(offset<(1<<24))
	{
		if(shrink_del(i->data[14], offset, 2))
			i->data[14]=-1;
		return;
	}
	fprintf(stderr, "inode->blocks corrupted\n");
}

/*
 * tree view backend
 */
void tree_backend(uint32_t par, char *buffer, size_t len)
{
	long ed=get_inode(par)->size/128;
	if(ed==0)return;
	for(long i=0;i<ed;++i)
	{
		struct dir_entry *ent=get_dir_entry(par, i);
		printf("%s", buffer);
		printf("%s", (i==ed-1?"└":"├"));
		printf("──%s\n", ent->name);
		
		if(get_inode(ent->inode)->mode==MODE_DIR)
		{
			const char a[]="│  ";
			const char b[]="   ";
			if(!strcmp(ent->name, "."))continue;
			if(!strcmp(ent->name, ".."))continue;
			strcpy(buffer+len, (i==ed-1?b:a));
			len+=strlen((i==ed-1?b:a));
			tree_backend(ent->inode, buffer, len);
			len-=strlen((i==ed-1?b:a));
			buffer[len]=0;
		}
	}
}
 
/*
 * par=inode of the dir
 * draw a nice tree of folder name
 * not check inode->mode
 */          
void tree_view(uint32_t par)
{                       
	size_t len=0;
	char buffer[1024];
	buffer[len]=0;
	printf("%s\n", par==0?"/":".");
   tree_backend(par, buffer, len);
}

/*
 * coi inode nhu 1 file
 * free du lieu cua ->data
 * free inode cell
 */
void inode_free(uint32_t ino)
{
	uint32_t c=get_inode(ino)->blocks;
	for(uint32_t i=0;i<c;++i)
		inode_shrink(ino);
	free_inode_cell(ino);
}

/*
 * Dem tat ca cac block ma inode dang quan ly
 * ino - so hieu inode
 */
uint32_t count_all_block(uint32_t ino)
{
	uint32_t data_count=get_inode(ino)->blocks;
	uint32_t map_count;
	if(data_count<12)map_count=0;
	else if(data_count<12+256)map_count=1;
	else if(data_count<12+256+65536)
	{

		uint32_t k=data_count-12-256;
		map_count=1+1+(k/256)+(k%256!=0);
	}
	else 
	{
		map_count=1+1+256+1;
		uint32_t k=data_count-12-256-65536;
		map_count+=k/256+(k%256!=0)+k/65536+(k%65536!=0);
	}
	return map_count+data_count;
}
 /* usage:tinh size cua dir
 * argv: ino so hieu inode
 * return: size cua inode
 */
uint32_t get_dir_size(uint32_t ino){
	uint32_t num_dir=get_inode(ino)->size/128;
	if (get_inode(ino)->mode!=MODE_DIR)
		return -1;
	uint32_t size=0;
	for (int i=0;i<num_dir;i++){
		struct dir_entry dir_tmp=*get_dir_entry(ino,i);
		struct inode inode_tmp=*get_inode(dir_tmp.inode);
		if(!strcmp(".", dir_tmp.name))continue;
		if(!strcmp("..", dir_tmp.name))continue;
		if (inode_tmp.mode==MODE_NOR)
			size+=inode_tmp.size;
		if (inode_tmp.mode==MODE_DIR)
			size+=get_dir_size(dir_tmp.inode);
	}
	return size;
}

void copy_file_nocheck(uint32_t srcino, uint32_t desino, const char *name)
{
 	struct inode buf=*get_inode(srcino);
	uint32_t ino=inode_alloc();
	struct inode *new=get_inode(ino);
	inode_init(new, MODE_NOR);
	new->size=buf.size;
	inode_expand(ino, buf.blocks);
	for(uint32_t i=0;i<buf.blocks;++i)
	{
		char b[1024];
		memcpy(b, get_file_block(i, buf.data), 1024);
		memcpy(get_file_block(i, new->data), b, 1024);
	}
	add_dir_entry(desino, ino, name);    
}
 
void dir_usage(uint32_t ino, uint32_t *block, uint32_t *inode)
{
	*inode+=1;
	*block+=count_all_block(ino);
	if(get_inode(ino)->mode==MODE_DIR)
	{
		uint32_t ed=get_inode(ino)->size/128;
		for(uint32_t i=0;i<ed;++i)
		{
			if(!strcmp(get_dir_entry(ino, i)->name, "."))continue;
			if(!strcmp(get_dir_entry(ino, i)->name, ".."))continue;
			dir_usage(get_dir_entry(ino, i)->inode, block, inode);
		}
	}
}
