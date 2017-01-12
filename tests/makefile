all: vfs.o vfsPathParser.o vfs_safe.o
	g++ -std=c++11 vfs_safe.o terminalTest.cc vfsPathParser.o vfs.o ../virtualFileSystem/virtualFileSystemWrapper.cc ../virtualFileSystem/hiredis/*.o

vfs.o: ../virtualFileSystem/vfs.c
	gcc -g -O0 -c ../virtualFileSystem/vfs.c

vfsPathParser.o: ../virtualFileSystem/vfsPathParser.c
	gcc -g -O0 -c ../virtualFileSystem/vfsPathParser.c

vfs_safe.o: ../virtualFileSystem/vfs_safe.cc
	g++ --std=c++11 -g -O0 -c ../virtualFileSystem/vfs_safe.cc
