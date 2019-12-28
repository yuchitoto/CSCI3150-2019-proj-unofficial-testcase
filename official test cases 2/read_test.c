#include "call.h"

int main (int argc, char *argv[])
{
	//argv[1]= A new file in SFS with full pathname
	char filename[MAX_COMMAND_LENGTH]="/file1";

	/*
	Allocate a buf with MAX_FILE_SIZE.
	*/
	char buf[MAX_FILE_SIZE];

	int read_size;
	int test_inode=open_t(filename);
	//Start testi
	int offset_list[10] = {0};
	int count_list[10] =  {50};
	int expected[10] = {23};
	char* expected_bytes[10] = {"Hello World, csci-3150."};
	//read_t test
	for(int i = 0; i < 1; i++)
	{
		int cnt = count_list[i];
		int off = offset_list[i];
		printf("====case %d: read %d bytes from %d offest=======\n", i, cnt, off);
		read_size = read_t(test_inode, off, buf, cnt);
		buf[read_size] = '\0';
		printf("read size: %d\t expected: %d\n",read_size, expected[i]);
		printf("read bytes: %s\t expected: %s\n\n",buf, expected_bytes[i]);
	}
	return 0;
}