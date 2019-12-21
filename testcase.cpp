#include "call.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

superblock* read_sb_c(int fd) //copied from Tutorial 1
{
	superblock* sb = (superblock*)malloc(sizeof(superblock));
	int currpos=lseek(fd, SB_OFFSET, SEEK_SET);
	if(currpos<0){
		printf("Error: lseek()\n");
		return NULL;
	}

	//read superblock from disk
	int ret = read(fd, sb, sizeof(superblock));
	if(ret != sizeof (superblock) ){
		printf("Error: read()\n");
		return NULL;
	}
	return sb;
}

void print_sb_info(superblock* sb) //copied from Tutorial 1
{
	printf("the super block information:\n");
	printf("inode_offset:			%d\n", sb->inode_offset);
	printf("data_offset:			%d\n", sb->data_offset);
	printf("max_inode:			%d\n", sb->max_inode);
	printf("max_data_blk:			%d\n", sb->max_data_blk);
	printf("next_available_inode:		%d\n", sb->next_available_inode);
	printf("next_available_blk:		%d\n", sb->next_available_blk);
	printf("blk_size:			%d\n", sb->blk_size);
}

int main(int argc, char** argv)
{
  if(argc != 2)
  {
    cerr << "Usage: ./testcase [command]" << endl;
    cerr << "command\n" << "official: official test case\n" << "1: 1KB block size HD test\n" << endl;
    return 0;
  }
  system("g++ generator/generator.cpp -o hd_generator -lpthread -std=c++0x");
  if(strcmp(argv[1],"official")==0)
  {
    system("unzip ./official\\ test\\ cases/HD.zip");
    system("gcc official\\ test\\ cases/open_test.c call.c -I. -o open_t");
    system("./open_t");
    system("gcc official\\ test\\ cases/read_test.c call.c -I. -o read_t");
    system("./read_t");
    system("rm read_t open_t");
  }
  else if(strcmp(argv[1],"1")==0)
  {
    system("./hd_generator ./testcases/1kb");
    int fd = open("./HD",O_RDONLY);
    superblock *sb = read_sb_c(fd);
    print_sb_info(sb);
    char file[7][100] = {"/dir2", "/", "/dir3/dir6/dir7/file5", "/dir5", "/dir4/dir9/file", "/dir2/file4", "/dir4/dir9/dir10/dir11/file9"};
    int exp_ind[7] = {2,0,17,-1,-1,16,21};
    for(int i=0; i<7; i++)
    {
      int ind = open_t(file[i]);
      if(ind!=exp_ind[i])
      {
        cout << "For " << file[i] << " expects inode " << exp_ind[i] << " but returned " << ind << endl;
      }
    }

    close(fd);
  }

  return 0;
}
