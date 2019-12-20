#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../fast-cpp-csv-parser/csv.h"
#include "../superblock.h"
#include "../inode.h"
#include <ctime>

#define KB 1024
#define MB 1024*KB

superblock* read_sb(void);
void write_bs(int fd);
void write_sb(int fd, superblock *sb);
void init_inode(int fd, int inode_offset, int max_inode, int data_offset);
int* write_inode(int fd, superblock *sb);

int main(void)
{
  superblock *sb = read_sb();
  int fd = open("./HD", O_RDWR | O_CREAT | O_TRUNC);
  write_bs(fd);
  write_sb(fd, sb);
  init_inode(fd, sb->inode_offset, sb->max_inode, sb->data_offset);
  int *cur = write_inode(fd, sb);
  sb->next_available_inode = cur[0];
  sb->next_available_blk = cur[1];
  write_sb(fd, sb);
  close(fd);
  free(sb);
  return 0;
}

int* write_inode(int fd, superblock *sb)
{
  io::CSVReader<4> in("dir_tree.csv");
  in.read_header(io::ignore_extra_column, "Name", "Type", "Parent", "size");
  std::string name, tp, parent;
  int size;
  int *curpointer=(int*)malloc(2*sizeof(int)); //0 = inode 1 = datablk
  memset(curpointer,0,2*sizeof(int));
  std::vector<DIR_NODE> dir[sb->max_inode];
  inode i[sb->max_inode];
  string dir_name[sb->max_inode];
  DIR_NODE tmp;
  while(in.read_row(name, tp, parent, size))
  {
    if(tp.compare("dir")==0)
    {
      //save inode
      if(name.compare("/")==0)
      {
        dir_name[0] = name.cstr();
        i[0].i_number = 0;
        i[0].i_mtime = time(NULL);
        i[0].i_type = 1;
        i[0].i_size = 2*sizeof(DIR_NODE);
        i[0].file_num=2;
        tmp.dir = ".";
        tmp.inode_number = 0;
        dir[0].push_back(tmp);
        tmp.dir = "..";
        dir[0].push_back(tmp);
        curpointer[0]++;
      }
      else{
        dir_name[curpointer[0]] = name.cstr();
        i[curpointer[0]].i_number = curpointer[0];
        i[curpointer[0]].i_mtime = time(NULL);
        i[curpointer[0]].i_type = 1;
        i[curpointer[0]].i_size = 2*sizeof(DIR_NODE);
        i[curpointer[0]].file_num = 2;
        int parent_i = -1;
        for(int count=0; count<curpointer[0]; count++)
        {
          if(dir_name[count].compare(parent)==0)
          {
            parent_i=count;
            break;
          }
        }
        if (parent_i <0)
        {
          std::cerr << "parent not found for " << name << std::endl;
          exit(-1);
        }
        i[parent_i].i_size += sizeof(DIR_NODE);
        i[parent_i].file_num++;
        tmp.dir = name;
        tmp.inode_number = curpointer[0];
        dir[parent_i].push_back(tmp);
        tmp.dir = ".";
        dir[curpointer[0]].push_back(tmp);
        tmp.dir = "..";
        tmp.inode_number = parent_i;
        dir[curpointer[0]].push_back(tmp);
        curpointer[0]++;
      }
    }
    else if(tp.compare("file")==0)
    {
      //save inode
      dir_name[curpointer[0]] = name.cstr();
      i[curpointer[0]].i_number = curpointer[0];
      i[curpointer[0]].i_mtime = time(NULL);
      i[curpointer[0]].i_type = 0;
      i[curpointer[0]].i_size = size;
      i[curpointer[0]].file_num = 1;
      int parent_i = -1;
      for(int count=0; count<curpointer[0]; count++)
      {
        if(dir_name.compare(parent)==0)
        {
          parent_i = count;
          break;
        }
      }
      if(parent_i<0)
      {
        std::cerr << "parent not found for " << name << sdt::endl;
        exit(-1);
      }
      i[parent_i].i_size += sizeof(DIR_NODE);
      i[parent_i].file_num++;
      tmp.dir = name.cstr();
      tmp.inode_number = curpointer[0];
      dir[parent_i].push_back(tmp);
      curpointer[0]++;
    }
  }
  return curpointer;
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
