#include <iostream>
#include <fstream>
#include "../fast-cpp-csv-parser/csv.h"
#include "../superblock.h"

superblock* read_sb(void);

int main(void)
{
  superblock *sb = read_sb();
  std::ofstream hd_file;
  hd_file.open("HD", std::ofstream::out | std::ofstream::trunc);
  return 0;
}
