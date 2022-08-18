#include<stdlib.h>
#include<string.h>
#include "struct.h"
#include<stdio.h>
#define IS_NULL(x) x==NULL
#define MALLOC_ERROR 1
#define MUST_BE_DIRECTORY_ERROR 2
#define DIR_ENTRY_NOT_FOUND 
#define IS_DIRECTORY(x) x->mode == 1
#define CHECK_ERROR if(ERROR) exit(ERROR);
#define NUM_ENTRY_PER_BLOCK 8
struct parsed_filename {
    char *name;
    struct parsed_filename* next; 
};
int ERROR = 0;
struct parsed_filename* parse_directory(char* filepath) {
    int i = 1;
    while(filepath[i]=='/') i++;
    if(filepath[i]=='\0') return NULL;

    int j = i+1;
    while(filepath[j]!='\0'&&filepath[j]!='/') j++;
    int name_len = j-i;
    
    struct parsed_filename *tmp = malloc(sizeof(struct parsed_filename));
    if(tmp == NULL) {
        ERROR = MALLOC_ERROR;
        printf("malloc parsed_filename error\n");
        return NULL;
    } 

    tmp -> name = malloc(name_len + 1);
    strncpy(tmp->name, filepath+i, name_len);

    tmp -> next = parse_directory(filepath + j);

    if(ERROR) {
        free(tmp -> name);
        free(tmp);
    }
    return NULL;
}
void ps(char *s) {
    printf("%s\n", s);
}
struct current_path {
    char name[1025]; // max length 1024, 1 byte for NULL terminated sym
    char *offset; // current offset pointer at name
};

int get_file_inode(int inode_id, struct parsed_filename* current_filename, struct current_path* path)
{
    // check if is directory
    struct inode* current_inode = get_inode(inode_id);
    if(!DIRECTORY(current_inode)) {
        printf("%s is not a directory\n", path->name);
        ERROR = MUST_BE_DIRECTORY_ERROR;
        return -1;
    }
    // check if is the end
    if(current_filename == NULL) {
        return inode_id;
    }
    // find file in this dir
    uint32_t child_id = dir_find_name(current_inode, current_filename->name);
    
    if(child_id == -1) {
        ERROR = DIR_ENTRY_NOT_FOUND;
        printf("%s does not have entry named %s", path->name, current_filename->name);
        return -1;
    }
    /* update current path, used to debug */
    path -> offset[0] = '/';
    path -> offset ++;
    int len = strlen(current_filename -> name);
    strcpy(path->offset, current_filename->name);
    path -> offset += len;

    return get_file_inode(child_id, current_filename->next, path);

}
void list_dir(int inode_id, struct current_path* cwd) {
    struct inode* current_inode = get_inode(inode_id);
    if(!IS_DIRECTORY(current_inode)) {
        printf("%s is not a directory\n", cwd->name);
        ERROR = MUST_BE_DIRECTORY_ERROR;
        return NULL;
    }
    int num_blocks = current_inode->blocks;
    for (int i=0; i<num_blocks; ++i) {
        char* buff = get_file_block(inode_id, i);
        struct dir_entry* current_entry = buff;
        for (int j = 0; j<NUM_ENTRY_PER_BLOCK; ++j) {
            printf("%s", current_entry->name);
            current_entry ++;
        }
    }

}
// a path can not exceed 1024 byte
int main(int argc, char **argv) {
    check_command();

    struct parsed_filename* filename = parse_directory("/hello/hi/how");
    CHECK_ERROR;

    // remeber to set offset to name in cwd
    struct current_path* cwd = malloc(sizeof(struct current_path));
    memset(cwd -> name, 0, sizeof(cwd->name));
    cwd -> offset = cwd -> name;
    
    int file_inode_id = get_file_inode(0, filename);
    if(ERROR) {
        free(cwd);
        exit(ERROR);
    }

    list_dir(file_inode_id, cwd);
    CHECK_ERROR;

}
