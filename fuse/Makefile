
CFLAGS = -Wall -DFUSE_USE_VERSION=26 `pkg-config fuse --cflags`
LINKFLAGS = -Wall `pkg-config fuse --libs`

all: bin/examplefs

clean:
	rm -rf bin obj

bin: 
	mkdir -p bin

bin/examplefs: bin obj/redisfs.o obj/examplefs.o obj/wrap.o obj/main.o
	g++ -std=c++11 -g -lfuse $(LINKFLAGS) -o bin/examplefs obj/* ../virtualFileSystem/virtualFileSystemWrapper.cc  ../tests/*.o ../virtualFileSystem/hiredis/*.o -lfuse -lboost_system

obj:
	mkdir -p obj

obj/examplefs.o: obj examplefs.cc examplefs.hh
	g++ -std=c++11 -g $(CFLAGS) -c examplefs.cc -o obj/examplefs.o

obj/redisfs.o: obj redisfs.cc redisfs.hpp
	g++ -std=c++11 -g $(CFLAGS) -c redisfs.cc -o obj/redisfs.o

obj/main.o: obj main.c wrap.hh
	gcc -std=c++11 -g $(CFLAGS) -c main.c -o obj/main.o

obj/wrap.o: obj wrap.cc wrap.hh examplefs.hh
	g++ -std=c++11 -g $(CFLAGS) -c wrap.cc -o obj/wrap.o

