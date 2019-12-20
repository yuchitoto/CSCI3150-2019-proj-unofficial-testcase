#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <cstring>
#include <string>
#include <cmath>
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
void write_bs(int fd, superblock *sb);
void write_sb(int fd, superblock *sb);
void init_inode(int fd, int inode_offset, int max_inode, int data_offset);
void write_inode(int fd, superblock *sb);
void init_datablk(int fd, int data_offset, int blk_size, int pos);

int main(void)
{
  superblock *sb = read_sb();
  int fd = open("./HD", O_RDWR | O_CREAT | O_TRUNC);
  write_bs(fd, sb);
  init_inode(fd, sb->inode_offset, sb->max_inode, sb->data_offset);
  write_inode(fd, sb);
  write_sb(fd, sb);
  close(fd);
  free(sb);
  return 0;
}

void init_datablk(int fd, int data_offset, int blk_size, int pos)
{
  lseek(fd, data_offset+pos*blk_size, SEEK_SET);
  char *blk = (char*)malloc(blk_size);
  memset(blk,0,blk_size);
  write(fd,blk,blk_size);
}

void write_inode(int fd, superblock *sb)
{
  io::CSVReader<4> in("dir_tree.csv");
  in.read_header(io::ignore_extra_column, "Name", "Type", "Parent", "size");
  std::string name, tp, parent;
  int size;
  std::queue<DIR_NODE> dir[sb->max_inode];
  inode i[sb->max_inode];
  std::string dir_name[sb->max_inode];
  DIR_NODE tmp;
  while(in.read_row(name, tp, parent, size))
  {
    if(tp.compare("dir")==0)
    {
      //save inode
      if(name.compare("/")==0)
      {
        dir_name[0] = name.c_str();
        i[0].i_number = 0;
        i[0].i_mtime = time(NULL);
        i[0].i_type = 1;
        i[0].i_size = 2*sizeof(DIR_NODE);
        i[0].file_num=2;
        strcpy(tmp.dir ,".");
        tmp.inode_number = 0;
        dir[0].push(tmp);
        strcpy(tmp.dir, "..");
        dir[0].push(tmp);
        sb->next_available_inode++;
      }
      else{
        dir_name[sb->next_available_inode] = name.c_str();
        i[sb->next_available_inode].i_number = sb->next_available_inode;
        i[sb->next_available_inode].i_mtime = time(NULL);
        i[sb->next_available_inode].i_type = 1;
        i[sb->next_available_inode].i_size = 2*sizeof(DIR_NODE);
        i[sb->next_available_inode].file_num = 2;
        int parent_i = -1;
        for(int count=0; count<sb->next_available_inode; count++)
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
        strcpy(tmp.dir, name.c_str());
        tmp.inode_number = sb->next_available_inode;
        dir[parent_i].push(tmp);
        strcpy(tmp.dir,".");
        dir[sb->next_available_inode].push(tmp);
        strcpy(tmp.dir, "..");
        tmp.inode_number = parent_i;
        dir[sb->next_available_inode].push(tmp);
        sb->next_available_inode++;
      }
    }
    else if(tp.compare("file")==0)
    {
      if(size > (sb->blk_size*2 + sb->blk_size*(sb->blk_size/sizeof(inode))))
      {
        std::cerr << "warning: file: " << name << " too big" << std::endl;
      }
      //save inode
      dir_name[sb->next_available_inode] = name.c_str();
      i[sb->next_available_inode].i_number = sb->next_available_inode;
      i[sb->next_available_inode].i_mtime = time(NULL);
      i[sb->next_available_inode].i_type = 0;
      i[sb->next_available_inode].i_size = size;
      i[sb->next_available_inode].file_num = 1;
      int parent_i = -1;
      for(int count=0; count<sb->next_available_inode; count++)
      {
        if(parent.compare(dir_name[count])==0)
        {
          parent_i = count;
          break;
        }
      }
      if(parent_i<0)
      {
        std::cerr << "parent not found for " << name << std::endl;
        exit(-1);
      }
      i[parent_i].i_size += sizeof(DIR_NODE);
      i[parent_i].file_num++;
      strcpy(tmp.dir, name.c_str());
      tmp.inode_number = sb->next_available_inode;
      dir[parent_i].push(tmp);
      sb->next_available_inode++;
    }
  }

  for(int a=0; a<sb->next_available_inode; a++)
  {
    i[a].direct_blk[0] = sb->next_available_blk;
    init_datablk(fd, sb->data_offset, sb->blk_size, i[a].direct_blk[0]);
    i[a].direct_blk[1] = (i[a].i_size > sb->blk_size) ? ++sb->next_available_blk : sb->next_available_blk;
    init_datablk(fd, sb->data_offset, sb->blk_size, i[a].direct_blk[1]);
    i[a].indirect_blk = (i[a].i_size > sb->blk_size*2) ? ++sb->next_available_blk : sb->next_available_blk;
    init_datablk(fd, sb->data_offset, sb->blk_size, i[a].indirect_blk);
    lseek(fd, sb->inode_offset + a*sizeof(inode), SEEK_SET);
    write(fd, &i[a], sizeof(inode));
    sb->next_available_blk++;
    if(i[a].i_type == 1)
    {
      lseek(fd, sb->data_offset+i[a].direct_blk[0]*sb->blk_size, SEEK_SET);
      /*int cur = i[a].direct_blk[0];
      int tmp_size = 0;*/
      while(!dir[a].empty())
      {
        /*tmp_size+=sizeof(DIR_NODE);
        if(tmp_size > sb->blk_size)
        {
          cur++;
          if(cur != i[a].direct_blk[1])
          {

          }
        }*/
        write(fd, &dir[a].front(),sizeof(DIR_NODE));
        dir[a].pop();
      }
    }
    else
    {
      int num_blk_needed = (int)(ceil((double)i[a].i_size / (double)sb->blk_size));
      for(int k=2; k<num_blk_needed; k++)
      {
        init_datablk(fd, sb->data_offset, sb->blk_size, sb->next_available_blk++);
      }
    }
  }
  if(sb->next_available_blk-1 > sb->max_data_blk)
  {
    std::cerr << "number of data block exceeds limit" << std::endl;
  }
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

void write_bs(int fd, superblock *sb)
{
  char *boot = (char*)malloc(sb->inode_offset);
  memset(boot,0,sb->inode_offset);
  write(fd,boot,sb->inode_offset);
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
