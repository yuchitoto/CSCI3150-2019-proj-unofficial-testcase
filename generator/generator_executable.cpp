#include <iostream>
#include <fstream>
#include <cstring>
#include "generator.hpp"

#define PATH_LENGTH_LIMIT 100

using namespace std;

int main(int argc, char** argv)
{
  char *path = (char*)malloc(PATH_LENGTH_LIMIT);
  int gen_err;
  strcpy(path, "./");
  if(argc==2)
  {
    strcpy(path, argv[1]);
  }
  if((gen_err=generator(path))!=0)
  {
    cerr << "Error occured in generator function" << endl;
  }
  return 0;
}
