//TODO lam ham parse argc, va cd
#include<libgen.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<readline/readline.h>
#include<readline/history.h>
#include"util.h"

char cwd[1024];
uint32_t current_inode;

typedef struct 
{
	const char *name;
	int (*func)(char*);
} ftab;
int shell_exit(), clear_screen(), mkf(), mkd(), tree();
ftab table[]= {
	{"tree", tree},
	{"exit", shell_exit},
	{"clear", clear_screen},
	{"mkfile", mkf},
	{"mkdir", mkd},
	{NULL, NULL}
};
void init_readline_lib();
char *strip_white();
ftab *find_command();
int execute_line(char *);
int done;

int main()
{
	int fd=open("disk.img", O_RDWR);
	if(fd<0)
	{
		perror("open");
		exit(-1);
	}
	cache_init(fd);
	init_readline_lib();
	while(!done)
	{
		char *l, *s;

		l=readline("> ");
		if(!l)break;
		s=strip_white(l);
		if(*s)
		{
			add_history(s);
			execute_line(s);
		}
		free(l);
	}
	exit(0);
}

/* **************************************************************** */
/* **************************************************************** */
char *command_generator();
char **custom_completion();
char *directory_generator();

char *nothing(const char *text, int state){return NULL;};
void init_readline_lib()
{
  rl_readline_name = "myext2Shell";
  rl_completion_entry_function = directory_generator;
  rl_attempted_completion_function = custom_completion;
}

char **custom_completion(char *text, int start, int end)
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0) matches = rl_completion_matches(text, command_generator);
  return matches;
}

char *directory_generator(char *text, int state)
{
	static int list_index, list_count=0, len;
	static char **list=NULL;
	static uint32_t inode;

	if(!state)
	{
		for(int i=0;i<list_count;++i)
			free(list[i]);
		free(list);
		list_count=0;
		list=NULL;
		inode=current_inode;

		char *k=strdupa(text);
		char *q=k, *e=k+strlen(k)-1;
		while(*q=='/')++q;
		inode=q>k?0:inode;
		for(;e>=q;--e)
			if(*e=='/'){ *e=0;break; }
		if(e>=q)inode=traverse_path(inode, q);
		if(inode==-1)return NULL;

		len=strlen(text);
		list_index=0;
		list_count=(int)(get_inode(inode)->size/128);
		list=malloc(list_count*sizeof(void*));
		for(int i=0;i<list_count;++i)
		{
			struct dir_entry *ent=get_dir_entry(inode, i);
			int ch=get_inode(ent->inode)->mode == MODE_DIR;
			list[i]=malloc(strlen(ent->name)+1+ch);
			strcpy(list[i], ent->name);
			if(ch)strcat(list[i], "/");
		}
	}

	char *name=text+len;
	while(name>text&&name[0]!='/')--name;
	if(name[0]=='/')++name;
	int name_len=strlen(name);
	while(list_index<list_count)
	{
		char *fname=list[list_index++];
      if(strncmp(fname, name, name_len) == 0)
		{
			int l=strlen(fname);
			char *ret=malloc(1+len+l-name_len);
			if(!ret)
			{
				fprintf(stderr, "malloc() failed\n");
				return NULL;
			}
			if(fname[l-1]=='/') rl_completion_append_character='\0';
			strcpy(ret, text);
			strcat(ret, fname+name_len);
			return ret;
		}
	}
	return NULL;
}

char *command_generator(char *text, int state)
{
  static int list_index, len;
  const char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen(text);
    }

  /* Return the next name which partially matches from the command list. */
  while ((name=table[list_index].name))
    {
      list_index++;
      if (strncmp (name, text, len) == 0)
		{
			char *ret=strdup(name);
			if(!ret)fprintf(stderr, "malloc() failed\n");
			return ret;
		}
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

int execute_line(char *line)
{
  int i;
  ftab *command;
  char *word;
  i = 0;
  while (line[i] && whitespace (line[i])) i++;
  word = line + i;
  while (line[i] && !whitespace (line[i])) i++;
  if (line[i]) line[i++] = '\0';

  command = find_command (word);
  if (!command)
    {
      printf ("%s: Command not found\n", word);
      return -1;
    }

  while (whitespace (line[i]))
    i++;
  word = line + i;
  return ((*(command->func)) (word));
}
char *strip_white(char *string)
{
  char *s, *t;
  for (s = string; whitespace (*s); s++);
  if (*s == 0) return s;

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t)) t--;
  *++t = '\0';
  return s;
}
  
ftab* find_command(char *name)
{
  int i;
  for (i=0;table[i].name;i++)
    if (strcmp (name, table[i].name) == 0)
      return (&table[i]);
  return NULL;
}


/* Shell commands */
int shell_exit() { exit(0); }
int clear_screen(char *argv) { printf("\e[1;1H\e[2J"); return 1;}
int cd(const char *where)
{
	uint32_t t=traverse_path(current_inode, where+(where[0]=='/'));
	if(t==-1)return 0;
	current_inode=t;
	return 1;
	//TODO
}  

int mkf(char *argv)
{
	char *newfile=basename(argv);
	char *parrent=dirname(argv);

	if(!strcmp(newfile, "."))
	{
		puts("Filename ?");
		return 0;
	}   
	int slash=parrent[0]=='/';
	uint32_t c_ino=traverse_path(slash?0:current_inode, parrent+slash);  
	if(c_ino==-1)
	{
		//TODO: myerror();
		return 0;
	}
	if(dir_find_name(c_ino, newfile)!=-1)
	{
		printf("file %s da ton tai\n", newfile);
		return 0;
	}
	u_int32_t fino=inode_alloc();
	if(fino==-1)
	{
		//TODO: myerror();
		return 0;
	}
	struct inode *finode=get_inode(fino);
	inode_init(finode, MODE_NOR);
	if(!add_dir_entry(c_ino, fino, newfile))
	{
		//TODO: myerror();
		inode_free(fino);
		return 0;
	}	
	printf("Tao file thanh cong:%s\n",newfile);
	return 1;
}

int mkd(char *argv)
{
	char *newfile=basename(argv);
	char *parrent=dirname(argv);
	if(!strcmp(newfile, "."))
	{
		puts("Dirname ?");
		return 0;
	}
	int slash=parrent[0]=='/';
	uint32_t c_ino=traverse_path(slash?0:current_inode, parrent+slash);
	if(c_ino==-1)
	{
		//TODO: myerror();
		return 0;
	}
	if(dir_find_name(c_ino, newfile)!=-1)
	{
		printf("file %s da ton tai\n", newfile);
		return 0;
	} 
	uint32_t fino=inode_dir_alloc(c_ino);
	if(!fino)
	{
		//TODO: myerror();
		return 0;
	}

	if(!add_dir_entry(c_ino, fino, newfile))
	{
		//TODO: myerror();
		inode_free(fino);
	}
	printf("Tao folder '%s' thanh cong\n", newfile);
	return 1;
}

int tree(char *argv)
{
	int slash=argv[0]=='/';
	uint32_t c_ino=traverse_path(slash?0:current_inode, argv+slash);   
	if(c_ino==-1)
	{
		//TODO: myerror();
		return 0;
	}
	if(get_inode(c_ino)->mode!=MODE_DIR)
	{
		printf("%s is not a folder\n", basename(argv));
		return 0;
	}
	tree_view(c_ino);
	return 1;
}
