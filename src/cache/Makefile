CC = g++
CFLAGS_OBJ = -g -Wall `pkg-config fuse --cflags`

all:
	@echo "Compiling objects"
	$(CC) $(CFLAGS_OBJ) -c *.cc

cache:
	$(CC) $(CFLAGS_OBJ) -c cache.cc

clean:
	@echo "Deleting objects"
	rm -f *.o *~
