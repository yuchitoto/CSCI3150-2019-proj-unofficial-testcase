#ifndef _SUPER_BLOCK_H_
#define _SUPER_BLOCK_H_

#define SB_OFFSET  512 
#define INODE_OFFSET   4096 
#define DATA_OFFSET    10485760 
#define MAX_INODE      100
#define MAX_DATA_BLK   256000
#define BLOCK_SIZE   4096
#define MAX_NESTING_DIR 10
#define MAX_COMMAND_LENGTH  50
#define MAX_FILE_SIZE BLOCK_SIZE*1024 


typedef struct _super_block_
{
        int inode_offset;
        int data_offset;
        int max_inode;
        int max_data_blk;
        int next_available_inode;
        int next_available_blk;
        int blk_size;
}superblock;
#endif
