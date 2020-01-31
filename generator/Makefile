CC = g++
DEPSFLAGS = -std=c++0x -lpthread
CFLAGS = -I. -Igenerator

generator: generator_executable.o generator.o
	$(CC) -o generator generator_executable.o generator.o $(CFLAGS) $(DEPSFLAGS)

generator.o: generator.cpp
	$(CC) -c generator.cpp $(DEPSFLAGS)

clean:
	rm generator.o generator_executable.o

.PHONY: clean
