CC	 	= gcc
LD	 	= gcc
CFLAGS	= -w -g -O3 -lpthread -fopenmp
LDFLAGS	= 
DEFS 	=
all:	population  dual

population: main.o PacWar.o sort.o Utils.o
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o $@ $?

dual: dual.c PacWar.o sort.o Utils.o
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o $@ $?

clean:
	rm -f *.o
	rm -f *~
	rm -f dual population
	rm -rf *.dSYM
