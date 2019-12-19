#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "../fast-cpp-csv-parser/csv.h"
#include "../superblock.h"
#include "../inode.h"

superblock* read_sb(void);

int main(void)
{
  superblock *sb = read_sb();
  return 0;
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
