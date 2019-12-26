CC=g++

testcase: testcase.cpp call.c
	git submodule update --init --recursive
	$(CC) testcase.cpp call.c -o testcase -g3

clean:
	rm HD testcase hd_generator
