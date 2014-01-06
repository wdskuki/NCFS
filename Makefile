CC = g++
CC_JERASURE = gcc
CFLAGS_NCFS = -g -Wall -pthread `pkg-config fuse --libs`
CFLAGS_OBJ = -g -Wall `pkg-config fuse --cflags`
CFLAGS_UTIL = -g -Wall
LIB = -lpthread 

dir_src = src
dir_filesystem = $(dir_src)/filesystem
dir_config = $(dir_src)/config
dir_cache = $(dir_src)/cache
dir_coding = $(dir_src)/coding
dir_storage = $(dir_src)/storage
dir_utility= $(dir_src)/utility
dir_network = $(dir_src)/network
dir_gui = $(dir_src)/gui
dir_jerasure = $(dir_src)/jerasure

#dir_objs defines the directory of objects. objs defines object paths.
dir_objs = $(dir_filesystem) $(dir_cache) $(dir_coding) $(dir_storage) $(dir_gui) $(dir_network) $(dir_config) $(dir_utility)
objs = $(dir_filesystem)/ncfs.o $(dir_filesystem)/filesystem_utils.o $(dir_cache)/cache.o $(dir_storage)/storage.o $(dir_coding)/coding.o \
		$(dir_network)/connection.o $(dir_network)/network.o $(dir_gui)/*.o $(dir_config)/*.o \
		$(dir_jerasure)/reed_sol.o $(dir_jerasure)/cauchy.o $(dir_jerasure)/jerasure.o $(dir_jerasure)/galois.o

repair_objs = $(dir_utility)/recovery.o $(dir_filesystem)/filesystem_utils.o $(dir_cache)/*.o $(dir_storage)/*.o \
                $(dir_coding)/*.o $(dir_network)/*.o $(dir_gui)/*.o $(dir_config)/*.o \
                $(dir_jerasure)/*.o

ncfs: ${objs}
	$(CC_JERASURE) $(CFLAGS_OBJ) -c $(dir_jerasure)/cauchy.c -o $(dir_jerasure)/cauchy.o
	$(CC_JERASURE) $(CFLAGS_OBJ) -c $(dir_jerasure)/reed_sol.c -o $(dir_jerasure)/reed_sol.o
	$(CC_JERASURE) $(CFLAGS_OBJ) -c $(dir_jerasure)/jerasure.c -o $(dir_jerasure)/jerasure.o
	$(CC_JERASURE) $(CFLAGS_OBJ) -c $(dir_jerasure)/galois.c -o $(dir_jerasure)/galois.o
	@echo "Compiling ncfs"
	$(CC) $(CFLAGS_NCFS) -o ncfs $(objs)

$(objs):
	@for i in $(dir_objs); do \
	echo "Compiling in $$i"; \
	(cd $$i; $(CC) $(CFLAGS_OBJ) -c *.cc); done	

setup: $(dir_utility)/setup.c
	$(CC) $(CFLAGS_UTIL) $(dir_utility)/setup.c -o setup

recover: $(dir_utility)/recovery.cc 
	$(CC) $(CFLAGS_UTIL) $(repair_objs) -o recover  $(LIB)

remap: $(dir_utility)/remap.c
	$(CC) $(CFLAG_UTIL) $(dir_utility)/remap.c -o remap

benchmark: $(dir_utility)/benchmark.c
	$(CC) $(CFLAG_UTIL) $(dir_utility)/benchmark.c -o benchmark

clean:
	@echo "Deleting ncfs"
	rm -f ncfs *.o *~
	@echo "Deleting recover"
	rm -f recover
	rm -f $(dir_utility)/recovery.o
	@for i in $(objs); do \
	echo "Deleting $$i"; \
	(rm -f $$i); done
