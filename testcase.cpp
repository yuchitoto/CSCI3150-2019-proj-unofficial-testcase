#include "call.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

int main(int argc, char** argv)
{
  if(argc != 2)
  {
    cerr << "Usage: ./testcase [command]" << endl;
    cerr << "command\n" << "official: official test case\n" << "1: 1KB block size HD test\n" << endl;
    return 0;
  }
  if(strcmp(argv[1],"official")==0)
  {
    system("unzip ./official\\ test\\ cases/HD.zip");
    system("gcc official\\ test\\ cases/open_test.c call.c -I. -o open_t");
    system("./open_t");
    system("gcc official\\ test\\ cases/read_test.c call.c -I. -o read_t");
    system("./read_t");
    system("rm read_t open_t");
  }

  return 0;
}
