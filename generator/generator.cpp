#include <iostream>
#include <fstream>
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
void init_inode(int fd, int inode_offset, int max_inode, int data_offset);

int main(void)
{
  superblock *sb = read_sb();
  int fd = open("./HD", O_RDWR | O_CREAT | O_TRUNC);
  write_bs(fd);
  write_sb(fd, sb);
  init_inode(fd, sb->inode_offset, sb->max_inode, sb->data_offset);
  write_inode();
  close(fd);
  return 0;
}

void init_inode(int fd, int inode_offset, int max_inode, int data_offset)
{
  int curpos = lseek(fd, inode_offset, SEEK_SET);
  char *inode = (char*)malloc(data_offset);
  write(fd,inode,data_offset);
  free(inode);
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
  free(boot);
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

  std::ifstream ifs;
  ifs.open("superblock.csv", std::ifstream::in);
  if(ifs.is_open()==0)
  {
    ifs.close();
    return sb;
  }
  ifs.close();

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
