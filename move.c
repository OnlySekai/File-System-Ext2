#include<libgen.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include"struct.h"
#include"util.h"

int image;
   
int main(int argc, char **argv)
{
	if(argc!=4)
	{
		printf("Usage %s: <disk> <file> <dest>\n", argv[0]);
		exit(0);
	}

	image=open(argv[1], O_RDWR);
	if(image<0)
	{
		perror("open()");
		exit(-1);
	}
	cache_init(image);
	
	uint32_t fino=-1, cino=0;
	char *path=strdup(argv[2]);
	char *name=strtok(path, "/");
	if(name)
		while(1)
		{
			if(get_inode(cino)->mode!=1)
			{
				printf("%s is not a folder\n", name);
				free(name);
				exit(-1);
			}
			fino=cino;

			struct dir_entry *ent;
			uint32_t i=0;
			for(;i<get_inode(cino)->size/128;++i)
			{
				ent=get_dir_entry(cino, i);
				if(!strcmp(ent->name, name))
				{
					cino=ent->inode;
					goto founded;
				}
			}
			printf("Not found folder %s\n", name);
			exit(-1);

			founded:
			if(!(name=strtok(NULL, "/")))
			{
				uint32_t old_dir_id=i;
				uint32_t new_ino;
				const char *new_name=NULL;
				if(argv[3][strlen(argv[3])-1]=='/')
				{
					new_name=get_dir_entry(fino, old_dir_id)->name;
					new_ino=traverse_path(0, argv[3]);
				}
				else 
				{
					new_name=basename(argv[3]);
					char *dname=dirname(argv[3]);
					if(!strcmp(dname, "."))dname="";
					new_ino=traverse_path(0, dname);
				}
				if(new_ino==-1) exit(-1);
				if(dir_find_name(new_ino, new_name)!=-1)
				{
					printf("File %s existed\n", new_name);
					exit(-1);
				}

				if(!add_dir_entry(new_ino, cino, new_name))
					exit(-1);
				if(!remove_dir_entry(fino, old_dir_id))
					exit(-1);

				get_inode(new_ino)->atime=mytime();
				exit(0);
			}
		}
	else
	{
		printf("You cant move '/'\n");
		exit(-1);
	}
}
