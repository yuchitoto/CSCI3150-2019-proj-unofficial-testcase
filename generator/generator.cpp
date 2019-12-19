#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../fast-cpp-csv-parser/csv.h"
#include "../superblock.h"
#include "../inode.h"

#define KB 1024
#define MB 1024*KB

superblock* read_sb(void);
void write_bs(int fd);
void write_sb(int fd, superblock *sb);

int main(void)
{
  superblock *sb = read_sb();
  int fd = open("./HB", O_RDWR | O_CREAT | O_TRUNC);
  write_bs(fd);
  write_sb(fd, sb);
  return 0;
}

void write_sb(int fd, superblock *sb)
{
  lseek(fd, SB_OFFSET, SEEK_SET);
  write(fd,sb,sizeof(superblock));
}

void write_bs(int fd)
{
  char *boot = (char*)malloc(INODE_OFFSET);
  memset(boot,0,INODE_OFFSET);
  write(fd,boot,INODE_OFFSET);
}

superblock* read_sb(void)
{
  superblock *sb = (superblock*)malloc(sizeof(superblock));
  sb->inode_offset = INODE_OFFSET;
  sb->data_offset = DATA_OFFSET;
  sb->max_inode = MAX_INODE;
  sb->max_data_blk = MAX_DATA_BLK;
  sb->next_available_inode = 0;
  sb->next_available_blk = 0;
  sb->blk_size = BLOCK_SIZE;

  io::CSVReader<2> in("superblock.csv");
  std::string data_type;
  int data;
  while(in.read_row(data_type, data))
  {
    if(data_type.compare("inode_offset")==0)
    {
      sb->inode_offset = data;
    }
    else if(data_type.compare("data_offset")==0)
    {
      sb->data_offset = data;
    }
    else if(data_type.compare("max_inode")==0)
    {
      sb->max_inode = data;
    }
    else if(data_type.compare("max_data_blk")==0)
    {
      sb->max_data_blk = data;
    }
    else if(data_type.compare("next_available_inode")==0)
    {
      sb->next_available_inode = data;
    }
    else if(data_type.compare("next_available_blk")==0)
    {
      sb->next_available_blk = data;
    }
    else if(data_type.compare("blk_size")==0)
    {
      sb->blk_size = data;
    }
  }
  return sb;
}
