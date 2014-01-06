CC = g++
CFLAGS_TEST = -g -Wall `pkg-config --libs`
CFLAGS_OBJ = -g -Wall `pkg-config --cflags`

objs = cache_test.o cache.o utils.o disk.o

%.o: %.cc %.hh
	$(CC) $(CFLAGS_OBJ) -c $< -o $@

%.o: %.cc
	$(CC) $(CFLAGS_OBJ) -c $< -o $@

cache_test: ${objs} 
	@echo "Compiling cache_test"
	$(CC) $(CFLAGS_TEST) -lpthread -o cache_test $(objs)

clean:
	@echo "Deleting objects"
	rm -f *.o *~ cache_test
