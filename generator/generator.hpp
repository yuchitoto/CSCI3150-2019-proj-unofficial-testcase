#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#ifndef PATH_LENGTH_LIMIT
#define PATH_LENGTH_LIMIT 100
#endif
#define KB 1024
#define MB 1024*KB

#ifndef README_SIZE
#define README_SIZE 3229
#endif

/*
* create HD using testcase with specification using csv
* @param path of csv
* @return status of whether success or not, HD file should be printed
*/
int generator(char* path);

#endif //GENERATOR_HPP
