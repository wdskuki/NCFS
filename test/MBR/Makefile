CC = g++
CFLAGS = -g -O2 -Wall
LIBS =
OBJS = netcodec.o GaloisField.o 

all: main

%.o: %.cc %.hh
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cc %.h
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJS) main.cc
	$(CC) $(CFLAGS) -c main.cc
	$(CC) $(CFLAGS) main.o $(OBJS) -o main

clean:
	@rm -f *.o
	@rm -f main

backup:
	tar --directory ../ -zcvf ../networkcoding_`date +%y%m%d`.tar.gz networkcoding 
