#include"error.h"
int ERROR;
const char *error_name[]={
	[MUST_BE_DIRECTORY_ERROR]="Must be directory",
	[FILE_NOT_FOUND]="File name not found",
	[MALLOC_ERROR]="Malloc error",
	[INODE_NOT_FOUND]="INODE_NOT_FOUND"
};

