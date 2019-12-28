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

#ifndef PATH_LENGTH_LIMIT
#define PATH_LENGTH_LIMIT 100
#endif
#define KB 1024
#define MB 1024*KB

#ifndef README_SIZE
#define README_SIZE 3197
#endif

superblock* read_sb(char* path);
void write_bs(int fd, superblock *sb);
void write_sb(int fd, superblock *sb);
void init_inode(int fd, int inode_offset, int max_inode, int data_offset);
void write_inode(int fd, superblock *sb, char* path);
void init_datablk(int fd, int data_offset, int blk_size, int pos);

int main(int argc, char** argv)
{
  char *path = (char*)malloc(PATH_LENGTH_LIMIT);
  strcpy(path,"./");
  if(argc == 2)
  {
    strcpy(path, argv[1]);
  }
  superblock *sb = read_sb(path);
  int fd = open("./HD", O_RDWR | O_CREAT | O_TRUNC);
  write_bs(fd, sb);
  init_inode(fd, sb->inode_offset, sb->max_inode, sb->data_offset);
  write_inode(fd, sb, path);
  write_sb(fd, sb);
  close(fd);
  free(sb);
  free(path);
  return 0;
}

void init_datablk(int fd, int data_offset, int blk_size, int pos)
{
  lseek(fd, data_offset+pos*blk_size, SEEK_SET);
  char *blk = (char*)malloc(blk_size);
  memset(blk,0,blk_size);
  write(fd,blk,blk_size);
}

void write_inode(int fd, superblock *sb, char *path)
{
  //read from csv
  char csv_path[PATH_LENGTH_LIMIT+50];
  strcat(csv_path,path);
  strcat(csv_path,"/dir_tree.csv");
  io::CSVReader<4> in(csv_path);
  in.read_header(io::ignore_extra_column, "Name", "Type", "Parent", "size");
  std::string name, tp, parent;
  int size;

  //inode storage
  std::queue<DIR_NODE> dir[sb->max_inode];
  inode i[sb->max_inode];
  std::string dir_name[sb->max_inode];

  DIR_NODE tmp;
  while(in.read_row(name, tp, parent, size))
  {
    if(tp.compare("dir")==0) //directory
    {
      //save inode
      if(name.compare("/")==0)//root
      {
        dir_name[0] = name.c_str();
        i[0].i_number = 0;
        i[0].i_mtime = time(NULL);
        i[0].i_type = 1;
        i[0].i_size = 2*sizeof(DIR_NODE);
        i[0].file_num=2;

        //update dir mapping
        strcpy(tmp.dir ,".");
        tmp.inode_number = 0;
        dir[0].push(tmp);
        strcpy(tmp.dir, "..");
        dir[0].push(tmp);
        sb->next_available_inode++;
      }
      else //not root
      {
        dir_name[sb->next_available_inode] = name.c_str();
        i[sb->next_available_inode].i_number = sb->next_available_inode;
        i[sb->next_available_inode].i_mtime = time(NULL);
        i[sb->next_available_inode].i_type = 1;
        i[sb->next_available_inode].i_size = 2*sizeof(DIR_NODE);
        i[sb->next_available_inode].file_num = 2;

        //locate parent
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

        //update dir mapping
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
    else if(tp.compare("file")==0)//file
    {
      if(size > (sb->blk_size*2 + sb->blk_size*(sb->blk_size/sizeof(int))))
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

      //locate parent
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

      //update dir mapping
      i[parent_i].i_size += sizeof(DIR_NODE);
      i[parent_i].file_num++;
      strcpy(tmp.dir, name.c_str());
      tmp.inode_number = sb->next_available_inode;
      dir[parent_i].push(tmp);
      sb->next_available_inode++;
    }
  }

  //write into HD
  for(int a=0; a<sb->next_available_inode; a++)
  {
    //calculate and init pointer blocks
    i[a].direct_blk[0] = sb->next_available_blk;
    init_datablk(fd, sb->data_offset, sb->blk_size, i[a].direct_blk[0]);
    i[a].direct_blk[1] = (i[a].i_size > sb->blk_size) ? ++sb->next_available_blk : sb->next_available_blk;
    init_datablk(fd, sb->data_offset, sb->blk_size, i[a].direct_blk[1]);
    i[a].indirect_blk = (i[a].i_size > sb->blk_size*2) ? ++sb->next_available_blk : sb->next_available_blk;
    init_datablk(fd, sb->data_offset, sb->blk_size, i[a].indirect_blk);

    i[a].i_blocks = ceil((double)i[a].i_size / (double)sb->blk_size);
    i[a].i_blocks += (i[a].i_blocks>2)?1:0;

    i[a].file_num = dir[a].size();

    //write inode
    lseek(fd, sb->inode_offset + a*sizeof(inode), SEEK_SET);
    write(fd, &i[a], sizeof(inode));
    sb->next_available_blk++;

    //write dir mapping
    if(i[a].i_type == 1)//dir
    {
      lseek(fd, sb->data_offset+i[a].direct_blk[0]*sb->blk_size, SEEK_SET);
      /*int cur = i[a].direct_blk[0];
      int tmp_size = 0;*/
      while(!dir[a].empty())//write dir mapping for dir
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
    else//file
    {
      int num_blk_needed = (int)(ceil((double)i[a].i_size / (double)sb->blk_size));
      for(int k=2; k<num_blk_needed; k++)//skip the number of data_blks
      {
        init_datablk(fd, sb->data_offset, sb->blk_size, sb->next_available_blk++);
      }

      //std::cout << num_blk_needed << " " << sb->blk_size << " " << i[a].i_size << std::endl << std::endl;

      if(i[a].i_size==README_SIZE)
      {
        //std::cout << "found readme" << std::endl;
        int tmp_sz = i[a].i_size;
        int r = open("./README.md",O_RDONLY);
        char buf[sb->blk_size];
        if(num_blk_needed>0)
        {
          memset(buf,0,sb->blk_size);
          read(r,buf,(sb->blk_size<tmp_sz)?sb->blk_size:tmp_sz);
          lseek(fd,sb->data_offset+i[a].direct_blk[0]*sb->blk_size,SEEK_SET);
          write(fd,buf,sb->blk_size);
          //std::cout << buf << std::endl << std::endl;
          tmp_sz-=sb->blk_size;
          if(num_blk_needed>1)
          {
            memset(buf,0,sb->blk_size);
            read(r,buf,(sb->blk_size<tmp_sz)?sb->blk_size:tmp_sz);
            lseek(fd,sb->data_offset+i[a].direct_blk[1]*sb->blk_size,SEEK_SET);
            write(fd,buf,sb->blk_size);
            //std::cout << buf << std::endl << std::endl;
            tmp_sz-=sb->blk_size;
            if(num_blk_needed>2)
            {
              int extra_blk = num_blk_needed-2;
              int tmp_inode;
              lseek(fd,sb->data_offset+i[a].indirect_blk*sb->blk_size,SEEK_SET);
              for(int d=1;d<=extra_blk;d++)
              {
                tmp_inode = d+i[a].indirect_blk;
                write(fd,&tmp_inode,sizeof(int));
              }
              lseek(fd,sb->data_offset+(i[a].indirect_blk+1)*sb->blk_size,SEEK_SET);
              for(int d=0;d<extra_blk;d++)
              {
                memset(buf,0,sb->blk_size);
                read(r,buf,(sb->blk_size<tmp_sz)?sb->blk_size:tmp_sz);
                write(fd,buf,(sb->blk_size<tmp_sz)?sb->blk_size:tmp_sz);
                tmp_sz-=sb->blk_size;
                //std::cout << buf << std::endl << std::endl;
              }
            }
          }
        }
        close(r);
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

superblock* read_sb(char* path)
{
  char csv_path[PATH_LENGTH_LIMIT+50];
  strcat(csv_path,path);
  strcat(csv_path,"/superblock.csv");
  superblock *sb = (superblock*)malloc(sizeof(superblock));
  sb->inode_offset = INODE_OFFSET;
  sb->data_offset = DATA_OFFSET;
  sb->max_inode = MAX_INODE;
  sb->max_data_blk = MAX_DATA_BLK;
  sb->next_available_inode = 0;
  sb->next_available_blk = 0;
  sb->blk_size = BLOCK_SIZE;

  std::ifstream ifs;
  ifs.open(csv_path, std::ifstream::in);
  if(ifs.is_open()==0)
  {
    ifs.close();
    return sb;
  }
  ifs.close();

  io::CSVReader<2> in(csv_path);
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
