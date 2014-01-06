CC = g++
CFLAGS_OBJ = -g -Wall `pkg-config fuse --cflags`

all:
	@echo "Compiling objects"
	$(CC) $(CFLAGS_OBJ) -c *.cc

filesystem_utils.o:
	$(CC) $(CFLAGS_OBJ) -c filesystem_utils.cc

clean:
	@echo "Deleting objects"
	rm -f *.o *~
