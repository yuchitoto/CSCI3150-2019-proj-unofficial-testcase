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

#ifndef README_SIZE
#define README_SIZE 3191
#endif

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
    cerr << "command\n" << "official: official test case\n" << "1: 1KB block size HD test\n" << "4: 4KB block size HD test\n" << endl;
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
		system("unzip ./official\\ test\\ cases\\ 2/HD.zip");
		system("gcc official\\ test\\ cases\\ 2/read_test.c call.c -I. -o read_t");
		system("./read_t");
  }
  else if(strcmp(argv[1],"1")==0)
  {
    system("./hd_generator ./testcases/1kb");
    int fd = open("./HD",O_RDONLY);
    superblock *sb = read_sb_c(fd);
    print_sb_info(sb);
    char file[7][100] = {"/dir2", "/", "/dir3/dir6/dir7/file5", "/dir5", "/dir4/dir9/file", "/dir2/file4", "/dir4/dir9/dir10/dir11/file9"};
    int exp_ind[7] = {2,0,17,-1,-1,16,21};
    int offset[] = {0,10,23,1100,2000,5000,8000,10000,50000};
    int count[] = {1000,1000,2000,5000,10000,10000,20000,2000,1};
    int j;
		char buf[sb->blk_size*(2+sb->blk_size/sizeof(int))];
    for(int i=0; i<7; i++)
    {
      int ind = open_t(file[i]);
      if(ind!=exp_ind[i])
      {
        cout << "For " << file[i] << " expects inode " << exp_ind[i] << " but returned " << ind << endl;
      }
			if(exp_ind[i]<0)
				continue;
			int ans2[] = {1000,1000,1088,11,0};
			int ans5[] = {1000,1000,2000,5000,6976,3976,976,0,0};
			int ans6[] = {1000,1000,1291,214,0};
			int tmprd;

      switch(i)
      {
        case 2:

        for(j=0;j<5;j++)
        {
					memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
          tmprd = read_t(ind,offset[j],buf,count[j]);
          if(tmprd != ans2[j])
						cout << "case " << i << " read case " << j << " expected " << ans2[j] << " but returned " << tmprd << endl;
        }
				break;
        case 5:

				for(j=0;j<9;j++)
        {
					memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
          tmprd = read_t(ind,offset[j],buf,count[j]);
          if(tmprd != ans5[j])
						cout << "case " << i << " read case " << j << " expected " << ans5[j] << " but returned " << tmprd << endl;
        }
				break;
        case 6:

				for(j=0;j<5;j++)
        {
					memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
          tmprd = read_t(ind,offset[j],buf,count[j]);
          if(tmprd != ans6[j])
						cout << "case " << i << " read case " << j << " expected "<< ans6[j] << " but returned " << tmprd << endl;
        }
				break;
        default:
				tmprd = read_t(ind,0,buf,5);
				if(tmprd != -1)
					cout << "case " << i << " expected -1 but returned " << tmprd << endl;
      }
    }
		//cout << "fin normal" << endl;

		//test buf
		char test_read_path[50] = "/dir3/dir6/dir8/file8";
		int test_read_ind = open_t(test_read_path);
		//cout << "test_read_ind: " << test_read_ind << endl;

		memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
		int out_sz = read_t(test_read_ind,0,buf,README_SIZE);

		char *chk_str=(char*)malloc(sb->blk_size*(2+sb->blk_size/sizeof(int)));
		memset(chk_str,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));

		int rd = open("README.md",O_RDONLY);
		read(rd,chk_str,README_SIZE);

		/*if(strcmp(buf,chk_str)!=0)
		{
			cout << "wrong reading" << endl;
			cout << "string read: " << buf << endl;
			cout << "ans: " << chk_str << endl;
		}*/
		memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
		memset(chk_str,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));

		out_sz = read_t(test_read_ind,30,buf,21);
		lseek(rd,30,SEEK_SET);
		read(rd,chk_str,21);
		if(strcmp(buf,chk_str)!=0)
		{
			cout << "wrong reading" << endl;
			cout << "string read: " << buf << endl;
			cout << "ans: " << chk_str << endl;
		}
		free(chk_str);
		close(rd);

    close(fd);
  }
	else if(strcmp(argv[1],"4")==0)
  {
    system("./hd_generator ./testcases/4kb");
    int fd = open("./HD",O_RDONLY);
    superblock *sb = read_sb_c(fd);
    print_sb_info(sb);
    char file[7][100] = {"/dir2", "/", "/dir3/dir6/dir7/file5", "/dir5", "/dir4/dir9/file", "/dir2/file4", "/dir4/dir9/dir10/dir11/file9"};
    int exp_ind[7] = {2,0,17,-1,-1,16,21};
    int offset[] = {0,10,23,1100,2000,5000,8000,10000,50000};
    int count[] = {1000,1000,2000,5000,10000,10000,20000,2000,1};
    int j;
		char buf[sb->blk_size*(2+sb->blk_size/sizeof(int))];
    for(int i=0; i<7; i++)
    {
      int ind = open_t(file[i]);
      if(ind!=exp_ind[i])
      {
        cout << "For " << file[i] << " expects inode " << exp_ind[i] << " but returned " << ind << endl;
      }
			if(exp_ind[i]<0)
				continue;
			int ans2[] = {1000,1000,1088,11,0};
			int ans5[] = {1000,1000,2000,5000,6976,3976,976,0,0};
			int ans6[] = {1000,1000,1291,214,0};
			int tmprd;

      switch(i)
      {
        case 2:

        for(j=0;j<5;j++)
        {
					memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
          tmprd = read_t(ind,offset[j],buf,count[j]);
          if(tmprd != ans2[j])
						cout << "case " << i << " read case " << j << " expected " << ans2[j] << " but returned " << tmprd << endl;
        }
				break;
        case 5:

				for(j=0;j<9;j++)
        {
					memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
          tmprd = read_t(ind,offset[j],buf,count[j]);
          if(tmprd != ans5[j])
						cout << "case " << i << " read case " << j << " expected " << ans5[j] << " but returned " << tmprd << endl;
        }
				break;
        case 6:

				for(j=0;j<5;j++)
        {
					memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
          tmprd = read_t(ind,offset[j],buf,count[j]);
          if(tmprd != ans6[j])
						cout << "case " << i << " read case " << j << " expected "<< ans6[j] << " but returned " << tmprd << endl;
        }
				break;
        default:
				tmprd = read_t(ind,0,buf,5);
				if(tmprd != -1)
					cout << "case " << i << " expected -1 but returned " << tmprd << endl;
      }
    }
		//cout << "fin normal" << endl;

		//test buf
		char test_read_path[50] = "/dir3/dir6/dir8/file8";
		int test_read_ind = open_t(test_read_path);
		//cout << "test_read_ind: " << test_read_ind << endl;

		memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
		int out_sz = read_t(test_read_ind,0,buf,README_SIZE);

		char *chk_str=(char*)malloc(sb->blk_size*(2+sb->blk_size/sizeof(int)));
		memset(chk_str,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));

		int rd = open("README.md",O_RDONLY);
		read(rd,chk_str,README_SIZE);
		/*if(strcmp(buf,chk_str)!=0)
		{
			cout << "wrong reading" << endl;
			cout << "string read: " << buf << endl;
			cout << "ans: " << chk_str << endl;
		}*/

		memset(buf,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));
		memset(chk_str,0,sb->blk_size*(2+sb->blk_size/sizeof(int)));

		out_sz = read_t(test_read_ind,30,buf,21);
		lseek(rd,30,SEEK_SET);
		read(rd,chk_str,21);
		if(strcmp(buf,chk_str)!=0)
		{
			cout << "wrong reading" << endl;
			cout << "string read: " << buf << endl;
			cout << "ans: " << chk_str << endl;
		}
		free(chk_str);

		close(rd);

    close(fd);
  }

  return 0;
}
