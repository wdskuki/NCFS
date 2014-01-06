CC = g++
CFLAGS_OBJ = -g -Wall `pkg-config fuse --cflags`

all:
	@echo "Compiling objects"
	$(CC) $(CFLAGS_OBJ) -c *.cc

storage.o:
	$(CC) $(CFLAGS_OBJ) -c storage.cc

clean:
	@echo "Deleting objects"
	rm -f *.o *~
