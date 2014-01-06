CC = g++
CFLAGS_OBJ = -g -Wall `pkg-config fuse --cflags`

all:
	@echo "Compiling objects"
	$(CC) $(CFLAGS_OBJ) -c *.cc

coding.o:
	$(CC) $(CFLAGS_OBJ) -c coding.cc

clean:
	@echo "Deleting objects"
	rm -f *.o *~
