all: vfs.o vfsPathParser.o
	g++ -std=c++11 terminalTest.cc vfsPathParser.o vfs.o ../virtualFileSystem/virtualFileSystemWrapper.cc ../virtualFileSystem/hiredis/*.o

vfs.o: ../virtualFileSystem/vfs.c
	gcc -g -O0 -c ../virtualFileSystem/vfs.c

vfsPathParser.o: ../virtualFileSystem/vfsPathParser.c
	gcc -g -O0 -c ../virtualFileSystem/vfsPathParser.c

