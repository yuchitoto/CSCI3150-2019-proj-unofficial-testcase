CC=g++

testcase: testcase.cpp call.c
	$(CC) testcase.cpp call.c -o testcase

clean:
	rm HD testcase hd_generator
