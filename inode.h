#ifndef _INODE_H_
#define _INODE_H_
#include <time.h>

typedef struct _inode_ 
{
	int i_number;
	time_t i_mtime;
	int  i_type;
	int i_size;
	int i_blocks;
	int direct_blk[2];
	int indirect_blk;
	int file_num;
}inode;

typedef struct dir_mapping
{
	char dir[20];
	int inode_number;
}DIR_NODE;



#endif
