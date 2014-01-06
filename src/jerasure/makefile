CC = gcc
CFLAGS_OBJ = -g -Wall `pkg-config fuse --cflags`

all:
	@echo "Compiling objects"
	$(CC) $(CFLAGS_OBJ) -c *.c

galois:
	$(CC) $(CFLAGS_OBJ) -c galois.c

jerasure:
	$(CC) $(CFLAGS_OBJ) -c jerasure.c

reed_sol:
	$(CC) $(CFLAGS_OBJ) -c reed_sol.c

cauchy:
	$(CC) $(CFLAGS_OBJ) -c cauchy.c

liberation:
	$(CC) $(CFLAGS_OBJ) -c liberation.c

clean:
	@echo "Deleting objects"
	rm -f *.o *~

